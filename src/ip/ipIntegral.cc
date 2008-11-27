#include "ipIntegral.h"
#include "Tensor.h"

/////////////////////////////////////////////////////////////////////////
// An index for the 3D tensor is: [y * stride_h + x * stride_w + p * stride_p]

// ID_FOR_TYPE - compute the integral (image) for any 2D/3D tensor type!
#define II_FOR_TYPE(tensorInType, tensorOutType, inType, outType)                       \
{                                                                                       \
        if (input.nDimension() == 3)                                                    \
        {                                                                               \
                II_3D_FOR_TYPE(tensorInType, tensorOutType, inType, outType);           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
                II_2D_FOR_TYPE(tensorInType, tensorOutType, inType, outType);           \
        }                                                                               \
}

// II_2D_FOR_TYPE - compute the integral (image) for any 2D tensor type!
#define II_2D_FOR_TYPE(tensorInType, tensorOutType, inType, outType)                    \
{                                                                                       \
        const tensorInType* t_input = (tensorInType*)&input;                            \
	tensorOutType* t_output = (tensorOutType*)m_output[0];                          \
                                                                                        \
	const inType* src = t_input->t->storage->data + t_input->t->storageOffset;      \
	outType* dst = t_output->t->storage->data + t_output->t->storageOffset;         \
                                                                                        \
	const int stride_h = t_input->t->stride[0];	                                \
	const int stride_w = t_input->t->stride[1];	                                \
                                                                                        \
	const int width = m_inputSize.w;                                                \
	const int height = m_inputSize.h;                                               \
	const int n_planes = input.size(2);                                             \
                                                                                        \
        dst[0] = src[0];                                                                \
        for (int x = 1; x < width; x ++)                                                \
        {                                                                               \
                const int baseindex = x * stride_w;                                     \
                dst[baseindex] = dst[baseindex - stride_w] + src[baseindex];            \
        }                                                                               \
                                                                                        \
        for (int y = 1; y < height; y ++)                                               \
        {                                                                               \
                int baseindex = y * stride_h;                                           \
                outType line = src[baseindex];                                          \
                                                                                        \
                dst[baseindex] = dst[baseindex - stride_h] + line;                      \
                baseindex += stride_w;                                                  \
                                                                                        \
                for (int x = 1; x < width; x ++, baseindex += stride_w)                 \
                {                                                                       \
                        line += src[baseindex];                                         \
                        dst[baseindex] = dst[baseindex - stride_h] + line;              \
                }                                                                       \
        }                                                                               \
}

// II_3D_FOR_TYPE - compute the integral (image) for any 3D tensor type!
#define II_3D_FOR_TYPE(tensorInType, tensorOutType, inType, outType)                    \
{                                                                                       \
        const tensorInType* t_input = (tensorInType*)&input;                            \
	tensorOutType* t_output = (tensorOutType*)m_output[0];                          \
                                                                                        \
	const inType* src = t_input->t->storage->data + t_input->t->storageOffset;      \
	outType* dst = t_output->t->storage->data + t_output->t->storageOffset;         \
                                                                                        \
	const int stride_h = t_input->t->stride[0];	                                \
	const int stride_w = t_input->t->stride[1];	                                \
	const int stride_p = t_input->t->stride[2];	                                \
                                                                                        \
	const int width = m_inputSize.w;                                                \
	const int height = m_inputSize.h;                                               \
	const int n_planes = input.size(2);                                             \
                                                                                        \
	for (int p = 0; p < n_planes; p ++)                                             \
	{                                                                               \
		const int dp = p * stride_p;                                            \
                                                                                        \
		dst[dp] = src[dp];                                                      \
		for (int x = 1; x < width; x ++)                                        \
		{                                                                       \
			const int baseindex = x * stride_w + dp;                        \
			dst[baseindex] = dst[baseindex - stride_w] + src[baseindex];    \
		}                                                                       \
                                                                                        \
		for (int y = 1; y < height; y ++)                                       \
		{                                                                       \
			int baseindex = y * stride_h + dp;                              \
			outType line = src[baseindex];                                  \
                                                                                        \
			dst[baseindex] = dst[baseindex - stride_h] + line;              \
			baseindex += stride_w;                                          \
                                                                                        \
			for (int x = 1; x < width; x ++, baseindex += stride_w)         \
			{                                                               \
				line += src[baseindex];                                 \
				dst[baseindex] = dst[baseindex - stride_h] + line;      \
			}                                                               \
		}                                                                       \
	}                                                                               \
}

/////////////////////////////////////////////////////////////////////////

namespace Torch {

/////////////////////////////////////////////////////////////////////////
// Constructor

ipIntegral::ipIntegral()
	:	ipCore()
{
}

/////////////////////////////////////////////////////////////////////////
// Destructor

ipIntegral::~ipIntegral()
{
}

/////////////////////////////////////////////////////////////////////////
// Check if the input tensor has the right dimensions and type

bool ipIntegral::checkInput(const Tensor& input) const
{
	// Accept only 2D/3D tensors of any type
	if (	input.nDimension() != 2 &&
                input.nDimension() != 3)
	{
	        return false;
	}

	// Accept only tensors having the set input size
	if (	input.size(0) != m_inputSize.h ||
		input.size(1) != m_inputSize.w)
	{
	        return false;
	}

	// OK
	return true;
}

/////////////////////////////////////////////////////////////////////////
// Allocate (if needed) the output tensors given the input tensor dimensions

bool ipIntegral::allocateOutput(const Tensor& input)
{
        if (	m_output == 0 ||
		m_output[0]->nDimension() != input.nDimension() ||
		m_output[0]->size(0) != m_inputSize.h ||
		m_output[0]->size(1) != m_inputSize.w ||
		(input.nDimension() == 2 && m_output[0]->size(2) != input.size(2)))
	{
		cleanup();

		// Need allocation
		m_n_outputs = 1;
		m_output = new Tensor*[m_n_outputs];

		// The output tensor's type depends on the input tensor's type
                switch (input.getDatatype())
                {
                case Tensor::Char:      // Char         -> Int
                case Tensor::Short:     // Short        -> Int
                case Tensor::Int:       // Int          -> Int
                        m_output[0] = input.nDimension() == 3 ?
                                new IntTensor(m_inputSize.h, m_inputSize.w, input.size(2)) :
                                new IntTensor(m_inputSize.h, m_inputSize.w);
                        break;

                case Tensor::Float:    // Float         -> Double
                case Tensor::Double:   // Float         -> Double
                        m_output[0] = input.nDimension() == 3 ?
                                new DoubleTensor(m_inputSize.h, m_inputSize.w, input.size(2)) :
                                new DoubleTensor(m_inputSize.h, m_inputSize.w);
                        break;

                default:
                        return false;
                }

		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
// Process some input tensor (the input is checked, the outputs are allocated)

bool ipIntegral::processInput(const Tensor& input)
{
         switch (input.getDatatype())
        {
        case Tensor::Char:      // Char         -> Int
                II_FOR_TYPE(CharTensor, IntTensor, char, int);
                break;

        case Tensor::Short:     // Short        -> Int
                II_FOR_TYPE(ShortTensor, IntTensor, short, int);
                break;

        case Tensor::Int:       // Int          -> Int
                II_FOR_TYPE(IntTensor, IntTensor, int, int);
                break;

        case Tensor::Float:    // Float         -> Double
                II_FOR_TYPE(FloatTensor, DoubleTensor, float, double);
                break;

        case Tensor::Double:   // Float         -> Double
                II_FOR_TYPE(DoubleTensor, DoubleTensor, double, double);
                break;

        default:
                return false;
        }

	// OK
	return true;
}

/////////////////////////////////////////////////////////////////////////

}
