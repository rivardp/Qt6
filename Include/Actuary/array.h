// Array.h			Version 1.0 - March 2, 2015  - Add append()
//							1.1 - March 15, 2015 - Add non-Array and non-Matrix copy constructors
//							1.2 - August 1, 2015 - Completely change Matrix class
//							1.3 - December 4, 2015 - Add method to convert to long double for Table class
//                                                      2.0 - November 16, 2019 - Move over to Qt
//
// Template to create arrays of objects
//
// One-dimensional class name is Array
// Two-dimensional class name is Matrix

#ifndef ARRAY_H
#define ARRAY_H


// Start with one dimensional array

template <class T>
class Array       
{
public:
	// constructors
	Array(unsigned int itsSize = 0);
	Array(const Array &rhs);
	Array(const T (&stdArray)[], unsigned int cols);
	~Array();

	// operators
	Array<T>& operator=(const Array<T>&);
	T& operator[](unsigned int index);
	const T& operator[](unsigned int index) const;

	// methods
	unsigned int getSize() const;
	void reSize(unsigned int newSize);
	void append(const T&);
	Array<long double>& asLongDouble();

private:
	T *pArray;
	unsigned int itsSize;
};

// implement the Constructor
template <class T>
Array<T>::Array(unsigned int size) : itsSize(size)
{
	pArray = new T[itsSize];
	for (unsigned int i=0; i<itsSize; i++)
                pArray[i] = T(static_cast<unsigned int>(0));
}

// copy constructor
template <class T>
Array<T>::Array(const Array &rhs)
{
	itsSize = rhs.GetSize();
	pArray = new T[itsSize];
	for (unsigned int i = 0; i<itsSize; i++)
		pArray[i] = rhs[i];
}

// non-Array copy constructor
template <class T>
Array<T>::Array(const T (&stdArray)[], unsigned int cols)
{
	unsigned int numElements = 0;
	if (sizeof(stdArray) > 0)
		numElements = sizeof(stdArray) / sizeof(T);
	if (numElements >= cols)
		itsSize = cols;
	else
		itsSize = numElements;
	pArray = new T[itsSize];
	for (unsigned int i = 0; i<itsSize; i++)
		pArray[i] = stdArray[i];
}

// destructor
template <class T>
Array<T>::~Array()
{
	itsSize = 0;
	delete [] pArray;
        pArray = nullptr;
}

// assignment operator=
template <class T>
Array<T>& Array<T>::operator=(const Array &rhs)
{
	if (this == &rhs)
		return *this;
	delete [] pArray;
	itsSize = rhs.getSize();
	pArray = new T[itsSize];
	for (unsigned int i=0; i<itsSize; i++)
		pArray[i] = rhs[i];
	return *this;
}

// subscript operator[]
template <class T>
T& Array<T>::operator[](unsigned int index)
{
	if (index >= itsSize)
		return pArray[itsSize-1];
	else
		return pArray[index];
}

template <class T>
const T& Array<T>::operator[](unsigned int index) const
{
	if (index >= itsSize)
		return pArray[itsSize-1];
	else
		return pArray[index];
}

template <class T>
unsigned int Array<T>::getSize() const
{
	return itsSize;
}

template <class T>
void Array<T>::reSize(unsigned int newSize)
{
	unsigned int i;
	T *origPointers = pArray;
	pArray = new T[newSize];
	unsigned int nValidPointers = (newSize >= itsSize) ? itsSize : newSize;
    for (i=0; i<nValidPointers; i++)
		pArray[i] = origPointers[i];
	for (i = nValidPointers; i<newSize; i++)
                pArray[i] = T(static_cast<unsigned int>(0));
	itsSize = newSize;
	delete [] origPointers;
        origPointers = nullptr;
}

template <class T>
void Array<T>::append(const T& newItem)
{
	unsigned int i, newSize;
	T *origPointers = pArray;

	newSize = itsSize + 1;
	pArray = new T[newSize];
	for (i = 0; i < itsSize; i++)
		pArray[i] = origPointers[i];
	pArray[itsSize] = newItem;
	itsSize = newSize;
	delete[] origPointers;
        origPointers = nullptr;
}

template <class T>
Array<long double>& Array<T>::asLongDouble()
{
        Array<long double> ald(itsSize);
        for (unsigned int index = 0; index < itsSize; index++)
		ald[index] = pArray[index];
	return ald;
}

// Move on to two dimensional array => matrix

template <class T>
class Matrix       
{
public:
	
	// Constructors
	Matrix(unsigned int rowSize = 0, unsigned int colSize = 0);
	Matrix(const Matrix<T> &rhs);
	Matrix(const T *stdArray, unsigned int rowSize, unsigned int colSize);
	~Matrix();

	// Operators
	Matrix<T>& operator=(const Matrix<T>&);
	T* operator[](unsigned int index);
	const T* operator[](unsigned int index) const;

	// Methods
	unsigned int getRowSize() const;
	unsigned int getColSize() const;
	void reSize(unsigned int newRowSize, unsigned int newColSize);


private:
	T** pArray;					// pointer to the array of pointers which point to the first col of each row
	unsigned int itsRowSize;
	unsigned int itsColSize;
};

/******** Create a two dimensional dynamic array in continuous memory ******
*
* - Define the pointer holding the array
* - Allocate memory for the array (linear)
* - Allocate memory for the pointers inside the array
* - Assign the pointers inside the array the corresponding addresses
*   in the linear array
**************************************************************************/

// implement the Constructor
template <class T>
Matrix<T>::Matrix(unsigned int rowSize, unsigned int colSize) :
				  itsRowSize(rowSize), itsColSize(colSize)
{
	pArray = new T*[itsRowSize];		// array of pointers to the first item in each row - pointer math will do the rest

	// Linear memory allocation
	T* pMemoryPool = new T[itsRowSize * itsColSize];
	for (unsigned int i = 0; i < itsRowSize; i++)
		pArray[i] = pMemoryPool + i * itsColSize;

	// Set all values to zero
	for (unsigned int i = 0; i < itsRowSize; i++)
		for (unsigned int j = 0; j < itsColSize; j++)
			pArray[i][j] = 0;

	// Assign null values if size = 0;
	if ((itsRowSize == 0) || (itsColSize == 0))
	{
                pArray = nullptr;
                pMemoryPool = nullptr;
	}
}

// Copy constructor
template <class T>
Matrix<T>::Matrix(const Matrix<T> &rhs)
{
	itsRowSize = rhs.getRowSize();
	itsColSize = rhs.getColSize();

	pArray = new T*[itsRowSize];		// array of pointers to the first item in each row - pointer math will do the rest

	// Linear memory allocation
	T* pMemoryPool = new T[itsRowSize * itsColSize];
	for (unsigned int i = 0; i < itsRowSize; i++)
		pArray[i] = pMemoryPool + i * itsColSize;

	// Copy values
	for (unsigned int i = 0; i < itsRowSize; i++)
		for (unsigned int j = 0; j < itsColSize; j++)
			pArray[i][j] = rhs[i][j];

	// Assign null values if size = 0;
	if ((itsRowSize == 0) || (itsColSize == 0))
	{
                pArray = nullptr;
                pMemoryPool = nullptr;
	}

}

// non-Matrix copy constructor
template <class T>
Matrix<T>::Matrix(const T* stdArray, unsigned int rowSize, unsigned int colSize)
{
	// This function is reliant on rows and cols being correct

	itsRowSize = rowSize;
	itsColSize = colSize;

	pArray = new T*[itsRowSize];		// array of pointers to the first item in each row - pointer math will do the rest

	// Linear memory allocation
	T* pMemoryPool = new T[itsRowSize * itsColSize];
	for (unsigned int i = 0; i < itsRowSize; i++)
		pArray[i] = pMemoryPool + i * itsColSize;

	// Copy values
	for (unsigned int i = 0; i < itsRowSize; i++)
		for (unsigned int j = 0; j < itsColSize; j++)
			pArray[i][j] = stdArray[i][j];

	// Assign null values if size = 0;
	if ((itsRowSize == 0) || (itsColSize == 0))
	{
                pArray = nullptr;
                pMemoryPool = nullptr;
	}

}

// Implement the destructor
template <class T>
Matrix<T>::~Matrix()
{
	itsRowSize = itsColSize = 0;
	delete[] pArray[0];			// frees memoryPool
	delete[] pArray;			// frees all the pointers
        pArray = nullptr;
}

// assignment operator=
template <class T>
Matrix<T>& Matrix<T>::operator=(const Matrix<T> &rhs)
{
	if (this == &rhs)
		return *this;
	delete[] pArray[0];
	delete[] pArray;

	itsRowSize = rhs.getRowSize();
	itsColSize = rhs.getColSize();

	pArray = new T*[itsRowSize];		// array of pointers to the first item in each row - pointer math will do the rest

	// Linear memory allocation
	T* pMemoryPool = new T[itsRowSize * itsColSize];
	for (unsigned int i = 0; i < itsRowSize; i++)
		pArray[i] = pMemoryPool + i * itsColSize;

	// Copy values
	for (unsigned int i = 0; i < itsRowSize; i++)
		for (unsigned int j = 0; j < itsColSize; j++)
			pArray[i][j] = rhs[i][j];

	return *this;
}

// subscript operator[] - works on the first dimension
template <class T>
T* Matrix<T>::operator[](unsigned int index)
{
	return pArray[index];  // returns pointer to first element in row
}

template <class T>
unsigned int Matrix<T>::getRowSize() const
{
	return itsRowSize;
}

template <class T>
unsigned int Matrix<T>::getColSize() const
{
	return itsColSize;
}

template <class T>
void Matrix<T>::reSize(unsigned int newRowSize, unsigned int newColSize)
{
	T** origPointers = pArray;

	pArray = new T*[newRowSize];		// array of pointers to the first item in each row - pointer math will do the rest

	// Linear memory allocation
	T* pMemoryPool = new T[newRowSize * newColSize];
	for (unsigned int i = 0; i < newRowSize; i++)
		pArray[i] = pMemoryPool + i * newColSize;

	unsigned int nValidRowPointers = (newRowSize >= itsRowSize) ? itsRowSize : newRowSize;
	unsigned int nValidColPointers = (newColSize >= itsColSize) ? itsColSize : newColSize;
    for (unsigned int i=0; i<nValidRowPointers; i++)
		for (unsigned int j=0; j<nValidColPointers; j++)
			pArray[i][j] = origPointers[i][j];
	for (unsigned int i = nValidRowPointers; i<newRowSize; i++)
		for (unsigned int j = nValidColPointers; j<newColSize; j++)
			pArray[i][j] = 0;
	itsRowSize = newRowSize;
	itsColSize = newColSize;

	// Assign null values if size = 0;
	if ((itsRowSize == 0) || (itsColSize == 0))
	{
                pArray = nullptr;
                pMemoryPool = nullptr;
	}

	if (origPointers)
	{
		delete[] origPointers[0];
		delete[] origPointers;
                origPointers = nullptr;
	}
}

#endif
