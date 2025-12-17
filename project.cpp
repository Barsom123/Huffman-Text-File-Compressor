#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<new>

using namespace std;

struct HuffmanNode {
    char data;
    long long freq;
    HuffmanNode* left, * right;
    HuffmanNode(char data, unsigned freq) {
        left = right = NULL;
        this->data = data;
        this->freq = freq;
    }
};
struct HeapNode
{
    HuffmanNode* huffNode;
};

class Heap
{
private:
    HeapNode* arr;
    int size; //Actual size of the dynamic array
    int capacity;  //Max size of the dynamic array

    //Helper Functions:To get the index of parent,left and right child
    int getLeftChildIndex(int parentIndex)
    {
        return 2 * parentIndex + 1;
    }
    int getRightChildIndex(int parentIndex)
    {
        return 2 * parentIndex + 2;
    }
    int getParentIndex(int childIndex)
    {
        return (childIndex - 1) / 2;
    }

    //Helper Functions:To check if parent,left and right child are already in the array
    bool hasLeftChild(int parentIndex)
    {
        if (getLeftChildIndex(parentIndex) < size)
            return true;
        return false;
    }

    bool hasRightChild(int parentIndex)
    {
        if (getRightChildIndex(parentIndex) < size)
            return true;
        return false;
    }
    bool hasParent(int childIndex)
    {
        if (getParentIndex(childIndex) >= 0)
            return true;
        return false;
    }

    //Helper Function:To check if there is enough space + Adjust heap otherwise
    void ensureHeapCapacity()
    {
        //if the actual size is smaller than the full capacity then there is enough space
        if (size < capacity)
            return;

        //OtherWise:
        HeapNode* newArr = new HeapNode[capacity * 2];

        if (newArr != nullptr)
        {
            capacity *= 2;

            for (int i = 0; i < size; i++)
                newArr[i] = arr[i];

            delete[]arr;
            arr = newArr;

        }
    }

    void swap(int i1, int i2)
    {
        HeapNode temp = arr[i1];
        arr[i1] = arr[i2];
        arr[i2] = temp;

    }
    //Helper Function:To make sure that the smallest value is always at first
    void heapifyUp()
    {
        int index = size - 1;

        while (hasParent(index) && arr[getParentIndex(index)].huffNode->freq > arr[index].huffNode->freq)
        {
            //If true,then we must swap
            swap(getParentIndex(index), index);
            index = getParentIndex(index);
        }
    }

    //Helper Function:To make sure that the new root is the right order
    void heapifyDown()
    {
        int index = 0;
        while (hasLeftChild(index))
        {
            int smallestChildIndex = getLeftChildIndex(index);
            if (hasRightChild(index) && arr[getRightChildIndex(index)].huffNode->freq < arr[smallestChildIndex].huffNode->freq)
                smallestChildIndex = getRightChildIndex(index);

            if (arr[index].huffNode->freq < arr[smallestChildIndex].huffNode->freq)
                break;
            else
            {
                swap(index, smallestChildIndex);
                index = smallestChildIndex;
            }

        }
    }
public:
    Heap()
    {
        size = 0;
        capacity = 256;
        arr = new HeapNode[capacity];
    }

    ~Heap()
    {
        delete[]arr;
    }
    int getSize() {
        return size;
    }
    void Add(HuffmanNode* node)
    {
        //1st step:we need to check if there is enough space to add a new item
        ensureHeapCapacity();

        //2nd step:we add the new item
        arr[size].huffNode = node;  //Last Index => O(1)
        size++;

        //3rd step:we ensure that heap characteristic is done after adding the new item (Min Heap)
        heapifyUp();
    }

    bool Poll(HuffmanNode*& node)
    {
        //Check if the array is empty
        if (size == 0)
            return false;
        node = arr[0].huffNode;

        //We assume that the last index is the new root
        arr[0] = arr[size - 1];
        size--;
        heapifyDown();
        return true;

    }
};
void readFileAndCountFrequencies(const char* filename, long long* freq) {
    FILE* inputFile = fopen(filename, "rb");
    if (inputFile == NULL) {
        printf("Error: Cannot open file!\n");
        return;
    }
    int ch;
    while ((ch = fgetc(inputFile)) != EOF) {
        if (ch < 128) {
            freq[ch]++;
        }
    }
    fclose(inputFile);
    FILE* freqFile = fopen("frequency_table.txt", "w");
    if (freqFile == NULL) {
        printf("Error: Cannot open output file!\n");
        return;
    }
    // store frequency table in output file
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            if (i >= 32 && i <= 126) {
                fprintf(freqFile, "'%c' : %lld\n", i, freq[i]);
            }
            else {
                fprintf(freqFile, "ASCII %d : %lld\n", i, freq[i]);
            }
        }
    }
    fclose(freqFile);

}

class HuffmanTree {
public:
    HuffmanNode* root;
    HuffmanTree() {
        root = NULL;
    }
    void printCodes() {
        string codes[256];
        getcodes(root, "", codes);
        for (int i = 0; i < 256; i++) {
            if (!codes[i].empty()) {
                printf("%c: %s\n", i, codes[i].c_str());
            }
        }
    }
    void getCodesArray(string codes[]) {
        getcodes(root, "", codes);
    }
    void destructorHelper(HuffmanNode* node) {
        if (node == NULL) return;
        destructorHelper(node->left);
        destructorHelper(node->right);
        delete node;
    }

    ~HuffmanTree() {
        destructorHelper(root);
    }
private:
    void getcodes(HuffmanNode* root, string code, string codes[]) {
        if (root == NULL)
        {
            return;
        }

        if (root->left == NULL && root->right == NULL) {
            codes[root->data] = code;
            return;
        }
        getcodes(root->left, code + "0", codes);
        getcodes(root->right, code + "1", codes);
    }

};
void buildheap(Heap*& H) {
    long long freq[256] = { 0 };
    readFileAndCountFrequencies("Text.txt", freq);
    printf("Character Frequencies:\n");
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            printf("Character: %c, Frequency: %lld\n", i, freq[i]);
            HuffmanNode* node = new HuffmanNode(i, freq[i]);
            H->Add(node);
        }
    }
}
void getrootofhuffman(HuffmanTree& ht, Heap*& H) {
    while (H->getSize() > 1) {
        HuffmanNode* left;
        HuffmanNode* right;

        H->Poll(left);
        H->Poll(right);

        HuffmanNode* parent = new HuffmanNode('#', left->freq + right->freq);
        parent->left = left;
        parent->right = right;

        H->Add(parent);
    }
    H->Poll(ht.root);
}
/*void compressFile(const char *inputFilename, const char *outputFilename, HuffmanTree &ht)
{
    // Implementation of file compression using Huffman coding
    // This function is a placeholder and needs to be implemented
    Heap *H = new Heap();
    buildheap(H);
    getrootofhuffman(ht, H);
    ht.printCodes();
    string codes[256];
    ht.getCodesArray(codes);
    FILE *inputFile = fopen(inputFilename, "rb");
    FILE *outputFile = fopen(outputFilename, "wb");
    if (inputFile == NULL || outputFile == NULL) {
        printf("Error: Cannot open input or output file!\n");
        return;
    }
     int ch;
    while ((ch = fgetc(inputFile)) != EOF) {
        if (ch < 128) {
            string code = codes[ch];
            // Write the code to output file (bit-level writing needed)
            // This part needs to be implemented
            fprintf(outputFile, "%s", code.c_str()); // Placeholder
        }
    }
    // Further implementation needed to read input file, encode using Huffman codes, and write to output file
    fclose(inputFile);
    fclose(outputFile);

}*/
void compressFile(const char* inputFilename, const char* outputFilename, HuffmanTree& ht)
{
    // Implementation of file compression using Huffman coding
    // This function is a placeholder and needs to be implemented
    char buffer = 0; // to store 8 bit before writing it as a byte in file
    int bitcountt = 0;
    char ch;
    Heap* H = new Heap();
    buildheap(H);
    getrootofhuffman(ht, H);
    ht.printCodes();
    string codes[256];
    ht.getCodesArray(codes);
    FILE* inputFile = fopen(inputFilename, "rb");
    FILE* outputFile = fopen(outputFilename, "wb");
    if (inputFile == NULL || outputFile == NULL) {
        printf("Error: Cannot open input or output file!\n");
        return;
    }
    while ((ch = fgetc(inputFile)) != EOF) {
        string code = codes[ch];
        for (int i = 0; code[i] != '\0'; i++)
        {
            // Shift buffer to the left to make space for the new bit
            buffer <<= 1;
            // Set the least significant bit if the current code bit is '1'
            if (code[i] == '1') buffer = buffer | 1;
            bitcountt++;
            if (bitcountt == 8)
            {
                printf("%d ", (unsigned char)buffer);
                fputc(buffer, outputFile);
                buffer = NULL;
                bitcountt = 0;
            }
        }

    }
    if (bitcountt > 0) {
        buffer <<= (8 - bitcountt);
        fputc(buffer, outputFile);
    }

    // Further implementation needed to read input file, encode using Huffman codes, and write to output file
    fclose(inputFile);
    fclose(outputFile);

}

int main() {
    HuffmanTree* ht = new HuffmanTree();
    compressFile("Text.txt", "compressed.com", *ht);
    delete ht;
    return 0;
}
