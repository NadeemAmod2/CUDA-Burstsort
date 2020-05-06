
using namespace std;

struct Node
{
	signed int stringsinbucket;
	union {
		char* bucket;
		unsigned int node;
	};
	char* firstunused;
	char* bottomofbucket;
};

struct Trie
{
	// the number of node blocks
	unsigned int numblocks;
	// the number of used nodes
	unsigned int numused;
	// the nodes	
	Node* nodes;
};

// Max bucket size is specifically chosen, see papers on Burstsort
#define MAXBUCKETSIZE 8192
// Initial trie size is arbitrary, but I choose it to be 8192 to
// reduce memory allocation calls. Now the memory allocation for the trie
// is the same as the buckets, i.e., double each time we run out.
#define INITIALTRIESIZE 1024

template<class T> class Burstsort
{
protected:
	T type;
	Trie trie;
	bool sorted;
	void burst(unsigned int);
	//void sort_bucket(unsigned int);
	void resize_bucket(unsigned int, unsigned int);
	void resize_trie(unsigned int);
	bool make_tailpointers(unsigned int);
	unsigned int maxlength;
	void vecswap2(char**, char**, int);
	char** med3func(char**, char**, char **, int);
	void inssort(char**, int, int);
	void ssort2(char**, int, int);
	void ssort2main(char**, int);
public:
	Burstsort();
	~Burstsort();
	void insert(wchar_t*);
	void sort();
	void print(bool, bool);
	void clear();
};

template <class T> Burstsort<T>::Burstsort()
{
	// Initialise the trie to having one level
	trie.numblocks = 0;
	trie.numused = 1;
	resize_trie(INITIALTRIESIZE);
	memset(trie.nodes, 0, type.alphabetsize() * 1 * sizeof(Node));
	// initialise variable
	maxlength = 0;
	sorted = false;
}

template <class T> Burstsort<T>::~Burstsort()
{
	clear();
	
}

template <class T> void Burstsort<T>::clear()
{
	// free buckets
	Stack<unsigned int> stack(maxlength);
	unsigned int currentnode = 0;
	for (;;) {
		for (;;) {
			// take care of the empty char symbol
			if ((currentnode % type.alphabetsize()) == 0) {
				currentnode++;
				continue;
			}
			// if this node points to another node then go further depthwise
			if (trie.nodes[currentnode].stringsinbucket == -1) {
				stack.push(currentnode);
				currentnode = trie.nodes[currentnode].node;
				continue;
			} else {
				// if we're in here then we've found a nonempty bucket
				delete [] trie.nodes[currentnode].bucket;
			}
		if ((++currentnode % type.alphabetsize()) == 0)
			break;
		}
		for (;;) {
			// if the stack is empty then we're done
			if (!stack.getsize()) {
				delete [] trie.nodes;
				return;
			}
			// else retrace our steps by poping the stack
			currentnode = stack.pop() + 1;
			if ((currentnode % type.alphabetsize()) != 0)
				break;
		}
	}
}

template <class T> void Burstsort<T>::insert(wchar_t* string)
{
	// make sure the string is valid to be inserted into the trie
	if (!type.isvalid(string))
		return;
	// update the maximum length
	unsigned int length = (unsigned int) wcslen(string);
	if (maxlength < length)
		maxlength = length;
	// this loop examines as many characters in the string as necessary
	unsigned int a = 0, node = 0;
	for(;;) {
		// if we've reached the end of the string then update the empty symbol string count
		if (string[a] == 0) {
			trie.nodes[node].stringsinbucket++;
			break;
		}
		// determine what bucket we need to insert the string into
		unsigned int symbolnode = node + type.chartonode((char)string[a]);
		// if there is no bucket, create one
		if (trie.nodes[symbolnode].stringsinbucket == 0)
			resize_bucket(1, symbolnode);
		// if this node points to a bucket and not another node then insert
		if (trie.nodes[symbolnode].stringsinbucket != -1) {
			unsigned int b = a + 1;
			for(;;) {
				// check whether we need to burst or resize the bucket
				if (trie.nodes[symbolnode].firstunused == trie.nodes[symbolnode].bottomofbucket) {
					unsigned int size = (unsigned int) (trie.nodes[symbolnode].bottomofbucket - trie.nodes[symbolnode].bucket);
					// if the size has not yet reached 8192 then double size
					if (size != MAXBUCKETSIZE)
						resize_bucket(size << 1, symbolnode);
					// otherwise burst the bucket
					else {
						burst(symbolnode);
						break;
					}
				}
				// otherwise copy the new character
				// not sure about pointers here, need to use reference to check
				// should be able to combine these two lines into one somehow
				trie.nodes[symbolnode].firstunused[0] = (char) string[b];
				trie.nodes[symbolnode].firstunused++;
				// if the end of the string then increase the number of strings and exit
				if (!string[b++]) {
					trie.nodes[symbolnode].stringsinbucket++;
					return;
				}
			}
		}
		// otherwise, go on to the next node
		node = trie.nodes[symbolnode].node;
		a++;
	}
}

// sort is essentially a depth-first search, which calls sort_bucket when it gets to a bucket
template <class T> void Burstsort<T>::sort()
{
	Stack<unsigned int> stack(maxlength);
	unsigned int currentnode = 0;
	for (;;) {
		for (;;) {
			// take care of the empty char symbol
			if ((currentnode % type.alphabetsize()) == 0) {
				currentnode++;
				continue;
			}
			// if this node points to another node then go further depthwise
			if (trie.nodes[currentnode].stringsinbucket == -1) {
				stack.push(currentnode);
				currentnode = trie.nodes[currentnode].node;
				continue;
			} else if (trie.nodes[currentnode].stringsinbucket > 1) {
				// if we're in here then we've found a nonempty bucket
				if (!make_tailpointers(currentnode)) {
					burst(currentnode);
					continue;
				} else {
					ssort2main((char**)trie.nodes[currentnode].firstunused, trie.nodes[currentnode].stringsinbucket);
				}
			}
		if ((++currentnode % type.alphabetsize()) == 0)
			break;
		}
		do {
			// if the stack is empty then we're done
			if (!stack.getsize()) {
				sorted = true;
				return;
			}
			// else retrace our steps by poping the stack
			currentnode = stack.pop() + 1;
		} while ((currentnode % type.alphabetsize()) == 0);
	}
}

template <class T> void Burstsort<T>::burst(unsigned int node)
{
	// create a new node block
	if (trie.numused == trie.numblocks)
		resize_trie(trie.numblocks * 2);
	trie.numused++;
	memset(&trie.nodes[type.alphabetsize() * (trie.numused - 1)], 0, type.alphabetsize() * sizeof(Node));

	// insert bucket's strings into new buckets
	int j = 0;
	for (int i = 0; i < trie.nodes[node].stringsinbucket; i++) {
		// if the empty string was in the bucket then update the empty char node
		if (!trie.nodes[node].bucket[j]) {
			trie.nodes[type.alphabetsize() * (trie.numused  - 1)].stringsinbucket++;
			j++;
			continue;
		}
		unsigned int symbolnode = type.alphabetsize() * (trie.numused - 1) + type.chartonode(trie.nodes[node].bucket[j++]);
		// if there is no bucket, create one
		if (trie.nodes[symbolnode].stringsinbucket == 0)
			resize_bucket(1, symbolnode);
		// if this node points to a bucket and not another node then insert
		if (trie.nodes[symbolnode].stringsinbucket != -1) {
			for(;;) {
				// check whether we need to burst or resize the bucket
				if (trie.nodes[symbolnode].firstunused == trie.nodes[symbolnode].bottomofbucket) {
					// double bucket capacity
					resize_bucket(unsigned int (trie.nodes[symbolnode].bottomofbucket - trie.nodes[symbolnode].bucket) << 1, symbolnode);
				}
				// copy characters (combine the following two lines??)
				*trie.nodes[symbolnode].firstunused = (char) trie.nodes[node].bucket[j];
				trie.nodes[symbolnode].firstunused++;
				// if the end of the string then increase the number of strings and exit
				if (!trie.nodes[node].bucket[j++]) {
					trie.nodes[symbolnode].stringsinbucket++;
					break;
				}
			}
		}
	}
	// update the old node
	trie.nodes[node].stringsinbucket = -1;
	delete [] trie.nodes[node].bucket;
	trie.nodes[node].node = type.alphabetsize() * (trie.numused - 1);
}

template <class T> void Burstsort<T>::print(bool printfile, bool printscreen)
{
	if (!printfile && !printscreen)
		return;
	Stack<unsigned int> stack(maxlength);
	wchar_t* string = new wchar_t[maxlength + 1];
	unsigned int currentnode = 0;
	wfstream* debugfile;
	if (printfile)
		debugfile = new wfstream(L"output.txt", ios::out);
	for (;;) {
		do {
			// take care of the empty char symbol
			if ((currentnode % type.alphabetsize()) == 0) {
				// make the string zero terminated
				string[stack.getsize()] = 0;
				// and print as many as there are with the empty char termining them
				for (unsigned int i = 0; i < trie.nodes[currentnode].stringsinbucket; i++) {
					if (printscreen)
						wcout << string << endl;
					if (printfile)
						*debugfile << string << endl;
				}						
				continue;
			}
			// if this node points to another node then go further depthwise
			if (trie.nodes[currentnode].stringsinbucket == -1) {
				// put the current character in the temporary string buffer
				string[stack.getsize()] = type.nodetochar(currentnode);
				// push the current node onto the stack so we can retrace our steps
				stack.push(currentnode);
				// set the current node to the one further into the trie 
				currentnode = trie.nodes[currentnode].node;
				continue;
			} else if (trie.nodes[currentnode].stringsinbucket != 0) {
				// if we're in here then we got to a bucket, so print all strings in bucket
				string[stack.getsize()] = type.nodetochar(currentnode);
				unsigned int j = 0; 
				for (unsigned int i = 0; i < trie.nodes[currentnode].stringsinbucket; i++) {
					unsigned int k = stack.getsize() + 1;
					if(!trie.nodes[currentnode].stringsinbucket)
						string[k] = 0;
					else if(sorted && trie.nodes[currentnode].stringsinbucket > 1) {
						j = 0;
						do {		
							string[k++] = (char)((char*)((char**)trie.nodes[currentnode].firstunused)[i])[j];
						} while ((char)((char*)((char**)trie.nodes[currentnode].firstunused)[i])[j++]);
					} else {
						do {
							string[k++] = (char) trie.nodes[currentnode].bucket[j];
						} while (trie.nodes[currentnode].bucket[j++]);
					}
					if (printscreen)
						wcout << string << endl;
					if (printfile)
						*debugfile << string << endl;
				}
			}
		} while ((++currentnode % type.alphabetsize()) != 0);
		do {
			// if the stack is empty then we're done
			if (!stack.getsize()) {
				delete [] string;
				if (printfile) {
					debugfile->close();
					delete debugfile;
				}
				return;
			}
			// else retrace our steps by poping the stack
			currentnode = stack.pop() + 1;
		} while ((currentnode % type.alphabetsize()) == 0);
	}
}

template <class T> bool Burstsort<T>::make_tailpointers(unsigned int node)
{
	unsigned int left = (unsigned int) (trie.nodes[node].bottomofbucket - trie.nodes[node].firstunused);
	unsigned int needed = trie.nodes[node].stringsinbucket * sizeof(char*);
	unsigned int used = (unsigned int) (trie.nodes[node].firstunused - trie.nodes[node].bucket);
	if (left < needed) {
		if ((used + needed) < MAXBUCKETSIZE) {
			// if we can expand the bucket then do that
			resize_bucket(used + needed, node);
		} else {
			// otherwise we will need to burst the bucket
			return false;
		}
	}
	unsigned int j = 0;
	char* i = trie.nodes[node].bucket;
	do {
		// if we're just starting off then insert a pointer to the tail string in the bucket
		if (i == trie.nodes[node].bucket)
			*(((char**) trie.nodes[node].firstunused) + j++) = i;
		// if the last character was an end of string then do the same
		else if (*(i - 1) == 0)
			*(((char**) trie.nodes[node].firstunused) + j++) = i;
	} while(++i < trie.nodes[node].firstunused);
	return true;
}

template <class T> void Burstsort<T>::resize_trie(unsigned int newnumblocks)
{
	Node* newnodes = new Node[newnumblocks * type.alphabetsize()];
	if (newnumblocks * type.alphabetsize() == 442368)
		bool test = true;
	if (newnumblocks > trie.numblocks ) {
		// scale up and zero the new nodes
		if (trie.numblocks) {
			memcpy(newnodes, trie.nodes, sizeof(Node) * trie.numblocks * type.alphabetsize());
			delete [] trie.nodes;
		}
	} else {
		// scale down
		memcpy(newnodes, trie.nodes, sizeof(Node) * newnumblocks * type.alphabetsize());
		delete [] trie.nodes;
	}		
	trie.nodes = newnodes;
	trie.numblocks = newnumblocks;
}

template <class T> void Burstsort<T>::resize_bucket(unsigned int newsize, unsigned int node)
{
	char* bucket = new char[newsize];
	if (trie.nodes[node].bottomofbucket - trie.nodes[node].bucket) {
		memcpy(bucket, trie.nodes[node].bucket, trie.nodes[node].bottomofbucket - trie.nodes[node].bucket);
		delete [] trie.nodes[node].bucket;
	}
	
	trie.nodes[node].firstunused = bucket + (unsigned int) (trie.nodes[node].firstunused - trie.nodes[node].bucket);
	trie.nodes[node].bucket = bucket;
	trie.nodes[node].bottomofbucket = bucket + newsize;
}

#ifndef min
#define min(a, b) ((a)<=(b) ? (a) : (b))
#endif

#define swap2(a, b) { t = *(a); *(a) = *(b); *(b) = t; }
#define ptr2char(i) (*(*(i) + depth))
#define med3(a, b, c) med3func(a, b, c, depth)

template <class T> void Burstsort<T>::vecswap2(char **a, char **b, int n)
{   
	while (n-- > 0) {
        char *t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

template <class T> char** Burstsort<T>::med3func(char **a, char **b, char **c, int depth)
{   
	int va, vb, vc;
    if ((va=ptr2char(a)) == (vb=ptr2char(b)))
        return a;
    if ((vc=ptr2char(c)) == va || vc == vb)
        return c;       
    return va < vb ?
          (vb < vc ? b : (va < vc ? c : a ) )
        : (vb > vc ? b : (va < vc ? a : c ) );
}

template <class T> void Burstsort<T>::inssort(char **a, int n, int d)
{   
	char **pi, **pj, *s, *t;
    for (pi = a + 1; --n > 0; pi++)
        for (pj = pi; pj > a; pj--) {
            // Inline strcmp: break if *(pj-1) <= *pj
            for (s=*(pj-1)+d, t=*pj+d; *s==*t && *s!=0; s++, t++)
                ;
            if (*s <= *t)
                break;
            swap2(pj, pj-1);
    }
}

template <class T> void Burstsort<T>::ssort2(char **a, int n, int depth)
{   
	int d, r, partval;
    char **pa, **pb, **pc, **pd, **pl, **pm, **pn, *t;
    if (n < 10) {
        inssort(a, n, depth);
        return;
    }
    pl = a;
    pm = a + (n/2);
    pn = a + (n-1);
    if (n > 30) { // On big arrays, pseudomedian of 9
        d = (n/8);
        pl = med3(pl, pl+d, pl+2*d);
        pm = med3(pm-d, pm, pm+d);
        pn = med3(pn-2*d, pn-d, pn);
    }
    pm = med3(pl, pm, pn);
    swap2(a, pm);
    partval = ptr2char(a);
    pa = pb = a + 1;
    pc = pd = a + n-1;
    for (;;) {
        while (pb <= pc && (r = ptr2char(pb)-partval) <= 0) {
            if (r == 0) { swap2(pa, pb); pa++; }
            pb++;
        }
        while (pb <= pc && (r = ptr2char(pc)-partval) >= 0) {
            if (r == 0) { swap2(pc, pd); pd--; }
            pc--;
        }
        if (pb > pc) break;
        swap2(pb, pc);
        pb++;
        pc--;
    }
    pn = a + n;
    r = min(pa-a, pb-pa);    vecswap2(a,  pb-r, r);
    r = min(pd-pc, pn-pd-1); vecswap2(pb, pn-r, r);
    if ((r = pb-pa) > 1)
        ssort2(a, r, depth);
    if (ptr2char(a + r) != 0)
        ssort2(a + r, pa-a + pn-pd-1, depth+1);
    if ((r = pd-pc) > 1)
        ssort2(a + n-r, r, depth);
}

template <class T> void Burstsort<T>::ssort2main(char **a, int n) { ssort2(a, n, 0); }