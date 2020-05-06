
template <class T> class Stack 
{
protected:
	T* stack;
	unsigned int capacity;
	unsigned int size;
public:
	Stack(unsigned int);
	~Stack(void);
	void push(T);
	T pop();
	unsigned int getsize();
};

template <class T> Stack<T>::Stack(unsigned int capacity)
{
	stack = new T[capacity];
	Stack::capacity = capacity;
	size = 0;
}

template <class T> Stack<T>::~Stack(void) {
	delete stack;
}

template <class T> void Stack<T>::push(T value) {
	if (size < capacity)
		stack[size++] = value;
}

template <class T> T Stack<T>::pop() {
	if (size)
		return stack[--size];
	else
		return 0;
}

template <class T> unsigned int Stack<T>::getsize() {
	return size;
}