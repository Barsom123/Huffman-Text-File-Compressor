#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>
void readFileAndCountFrequencies(const char *filename, long long *freq)
{
	// 1. read the input file and count frequencies
	FILE *inputFile = fopen(filename, "rb");
	if (inputFile == NULL)
	{
		printf("Error: Cannot open file: %s\n", filename);
		printf("Please make sure the file exists and the path is correct.\n");
		printf("compression process cannot continue.\n");
		printf("Exiting the program.\n");
		exit(1);
	}
	fseek(inputFile, 0, SEEK_END);
	long long fileSize = ftell(inputFile);
	if (fileSize > 4LL * 1024 * 1024 * 1024)
	{
		printf("Error: File size exceeds 4GB limit\n");
		printf("compression process cannot continue.\n");
		printf("Exiting the program.\n");
		fclose(inputFile);
		exit(1);
	}
	fseek(inputFile, 0, SEEK_SET);
	long long inputBufferSize = 1024 * 1024 * 4; // 1 MB buffer
	if (fileSize < inputBufferSize)
		inputBufferSize = fileSize;
	unsigned char *inputBuffer = new unsigned char[inputBufferSize];
	long bytesRead;
	while ((bytesRead = fread(inputBuffer, 1, inputBufferSize, inputFile)) > 0)
	{
		for (long long i = 0; i < bytesRead; i++)
		{
			freq[inputBuffer[i]]++;
		}
	}
	fclose(inputFile);

	// 2. print frequency table to console and save to output file
	FILE *freqFile = fopen("frequency_table.txt", "w");
	if (freqFile == NULL)
	{
		printf("Error: Cannot open output file: frequency_table.txt\n");
		exit(1);
	}
	// store frequency table in output file
	for (int i = 0; i < 256; i++)
	{
		if (freq[i] > 0)
		{
			printf("'%c' : %lld\n", i, freq[i]);
			fprintf(freqFile, "%d : %lld\n", i, freq[i]);
		}
	}
	fclose(freqFile);
	delete[] inputBuffer;
}

void getfilewithextenstion(char *filename, char *outputfilename, char *newextension)
{
	strcpy(outputfilename, filename);
	char *dot = strrchr(outputfilename, '.');
	if (dot)
		strcpy(dot, newextension);
	else
		strcat(outputfilename, newextension);
}

struct HuffmanNode
{
	unsigned char data;
	long long freq;
	HuffmanNode *left, *right;
	HuffmanNode(unsigned char data, long long freq)
	{
		left = right = NULL;
		this->data = data;
		this->freq = freq;
	}
};
struct HeapNode
{
	HuffmanNode *huffNode;
};

class Heap
{
private:
	HeapNode *arr;
	int size;	  // Actual size of the dynamic array
	int capacity; // Max size of the dynamic array

	// Helper Functions:To get the index of parent,left and right child
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

	// Helper Functions:To check if parent,left and right child are already in the array
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

	// Helper Function:To check if there is enough space + Adjust heap otherwise
	void ensureHeapCapacity()
	{
		// if the actual size is smaller than the full capacity then there is enough space
		if (size < capacity)
			return;

		// OtherWise:
		HeapNode *newArr = new HeapNode[capacity * 2];

		if (newArr != nullptr)
		{
			capacity *= 2;

			for (int i = 0; i < size; i++)
				newArr[i] = arr[i];

			delete[] arr;
			arr = newArr;
		}
	}

	void swap(int i1, int i2)
	{
		HeapNode temp = arr[i1];
		arr[i1] = arr[i2];
		arr[i2] = temp;
	}
	// Helper Function:To make sure that the smallest value is always at first
	void heapifyUp()
	{
		int index = size - 1;

		while (hasParent(index) && arr[getParentIndex(index)].huffNode->freq > arr[index].huffNode->freq)
		{
			// If true,then we must swap
			swap(getParentIndex(index), index);
			index = getParentIndex(index);
		}
	}

	// Helper Function:To make sure that the new root is the right order
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
		delete[] arr;
	}
	int getSize()
	{
		return size;
	}
	void Add(HuffmanNode *node)
	{
		// 1st step:we need to check if there is enough space to add a new item
		ensureHeapCapacity();

		// 2nd step:we add the new item
		arr[size].huffNode = node; // Last Index => O(1)
		size++;

		// 3rd step:we ensure that heap characteristic is done after adding the new item (Min Heap)
		heapifyUp();
	}

	bool Poll(HuffmanNode *&node)
	{
		// Check if the array is empty
		if (size == 0)
			return false;
		node = arr[0].huffNode;

		// We assume that the last index is the new root
		arr[0] = arr[size - 1];
		size--;
		heapifyDown();
		return true;
	}
	void buildheap(char *filename)
	{
		long long freq[256] = {0};
		readFileAndCountFrequencies(filename, freq);
		for (int i = 0; i < 256; i++)
		{
			if (freq[i] > 0)
			{
				// printf("Character: %c, Frequency: %lld\n", i, freq[i]);
				HuffmanNode *node = new HuffmanNode((unsigned char)i, freq[i]);
				Add(node);
			}
		}
	}
};

class HuffmanTree
{
public:
	HuffmanNode *root;
	HuffmanTree()
	{
		root = NULL;
	}
	void getandsavedcodes(char *inputfile, char *outs[256] = {0})
	{
		if (root->left == NULL && root->right == NULL)
		{
			// Special case: Only one unique character in the file
			codes[(unsigned char)root->data] = (char *)"0";
			printf("%c: %s\n", root->data, codes[(unsigned char)root->data]);
			return;
		}
		else
			getcodes(root, "");
		char codesFilename[256];
		getfilewithextenstion(inputfile, codesFilename, (char *)".cod");
		FILE *outputFile = fopen(codesFilename, "w");
		if (outputFile == NULL)
		{
			printf("Error: Cannot open output file: %s\n", codesFilename);
			exit(1);
		}
		for (int i = 0; i < 256; i++)
		{
			if (codes[i] != NULL)
			{
				fprintf(outputFile, "%d: %s\n", i, codes[i]);
				printf("%c: %s\n", i, codes[i]);
			}
			outs[i] = codes[i];
		}
		// Close the output file
		fclose(outputFile);
	}
	void getrootofhuffman(Heap *&H)
	{
		while (H->getSize() > 1)
		{
			HuffmanNode *left;
			HuffmanNode *right;
			H->Poll(left);
			H->Poll(right);
			HuffmanNode *parent = new HuffmanNode('#', left->freq + right->freq);
			parent->left = left;
			parent->right = right;
			H->Add(parent);
		}
		H->Poll(root);
	}
	void buildHuffmanTreeFromCodes()
	{
		destructorHelper(root); // Free existing tree if any
		this->root = new HuffmanNode('#', 0);
		for (int i = 0; i < 256; i++)
		{
			if (codes[i] != NULL) // 0101
			{
				HuffmanNode *current = this->root;
				for (int j = 0; codes[i][j] != '\0'; j++)
				{
					char bit = codes[i][j];
					if (bit == '0')
					{
						if (current->left == NULL)
						{
							current->left = new HuffmanNode('#', 0);
						}
						current = current->left;
					}
					else if (bit == '1')
					{
						if (current->right == NULL)
						{
							current->right = new HuffmanNode('#', 0);
						}
						current = current->right;
					}
				}
				current->data = (unsigned char)i;
			}
		}
	}
	~HuffmanTree()
	{
		destructorHelper(root);
		// Free allocated memory for codes
		for (int i = 0; i < 256; i++)
		{
			if (codes[i] != NULL)
				free(codes[i]);
		}
	}
	void setcodes(char *inputcodes[256])
	{
		for (int i = 0; i < 256; i++)
		{
			if (inputcodes[i] != NULL)
			{
				codes[i] = new char[strlen(inputcodes[i]) + 1];
				strcpy(codes[i], inputcodes[i]);
			}
		}
	}

private:
	char *codes[256] = {0};
	void getcodes(HuffmanNode *node, std::string code)
	{
		if (node == nullptr)
			return;

		// Leaf node
		if (node->left == nullptr && node->right == nullptr)
		{
			codes[(unsigned char)node->data] = new char[code.length() + 1];
			strcpy(codes[(unsigned char)node->data], code.c_str());
			return;
		}

		getcodes(node->left, code + "0");
		getcodes(node->right, code + "1");
	}
	void destructorHelper(HuffmanNode *node)
	{
		if (node == NULL)
			return;
		destructorHelper(node->left);
		destructorHelper(node->right);
		delete node;
	}
};

void compressFile(char *inputFilename, char *outputFilename)
{
	// 1. various variables needed for compression
	int bitcounter = 0;							 // to count number of bits in buffer
	int ch;										 // to read each character from input file
	long long originalFileSize;					 // to store original file size
	unsigned char padding = 0;					 // to store number of padding bits added in the last byte
	long paddingPos;							 // to store the position of padding byte in output file
	char *codes[256] = {0};						 // to store huffman codes for each character
	long inputBufferSize = 1024 * 1024 * 4;		 // 1 MB buffer
	long outputBufferSize = inputBufferSize * 2; // Output buffer can be larger due to encoding
	unsigned char *inputBuffer;					 // to read input file in chunks
	unsigned char *outputBuffer;				 // to store encoded output before writing to file
	unsigned char outByte = 0;					 // to store byte being formed for output
	long outIndex = 0;							 // index for output buffer
	long bytesRead;								 // number of bytes read from input file
	long long processedBytes = 0;				 // to track progress
	int lastPercent = -1;						 // to track last percentage printed
	HuffmanTree *ht = new HuffmanTree();
	// 2. Build Huffman Tree
	Heap *H = new Heap();
	H->buildheap(inputFilename);
	ht->getrootofhuffman(H);
	ht->getandsavedcodes(inputFilename, codes);

	// 3. Open input and output files
	FILE *inputFile = fopen(inputFilename, "rb");
	FILE *outputFile = fopen(outputFilename, "wb");
	if (inputFile == NULL || outputFile == NULL)
	{
		printf("Error: Cannot open input or output file! %s, %s\n", inputFilename, outputFilename);
		return;
	}
	// 4. get original file size
	fseek(inputFile, 0, SEEK_END);
	originalFileSize = ftell(inputFile);
	fseek(inputFile, 0, SEEK_SET);

	// 5. Write file extension to output file
	char *extension = strrchr(inputFilename, '.');
	unsigned char extlen = extension ? strlen(extension) : 0;
	fputc(extlen, outputFile);
	fwrite(extension, 1, extlen, outputFile);

	// 6. Reserve space for padding byte
	paddingPos = ftell(outputFile);
	fwrite(&padding, 1, 1, outputFile);

	// 7. Read input file, encode using Huffman codes, and write to output file

	// Adjust buffer sizes if file is smaller to avoid me
	if (originalFileSize < inputBufferSize)
	{
		inputBufferSize = originalFileSize;
		outputBufferSize = inputBufferSize * 2;
	}

	inputBuffer = new unsigned char[inputBufferSize];
	outputBuffer = new unsigned char[outputBufferSize];
	printf("Compressing process started.\n");
	while ((bytesRead = fread(inputBuffer, 1, inputBufferSize, inputFile)) > 0)
	{
		processedBytes += bytesRead;

		int percent = (int)((processedBytes * 100) / originalFileSize);

		if (percent != lastPercent)
		{
			printf("\rProcessed: %lld / %lld bytes (%d%%)", processedBytes, originalFileSize, percent);
			fflush(stdout);
			lastPercent = percent;
		}
		for (size_t j = 0; j < bytesRead; j++)
		{
			char *code = codes[inputBuffer[j]];

			for (int i = 0; code[i] != '\0'; i++)
			{
				outByte <<= 1;

				if (code[i] == '1')
					outByte |= 1;

				bitcounter++;

				if (bitcounter == 8)
				{
					outputBuffer[outIndex++] = outByte;
					if (outIndex == outputBufferSize)
					{
						fwrite(outputBuffer, 1, outputBufferSize, outputFile);
						outIndex = 0;
					}
					/// fputc(outByte, outputFile);
					outByte = 0;
					bitcounter = 0;
				}
			}
		}
	}

	if (bitcounter > 0)
	{
		padding = 8 - bitcounter;
		outByte <<= padding;
		outputBuffer[outIndex++] = outByte;
	}

	if (outIndex > 0)
	{
		fwrite(outputBuffer, 1, outIndex, outputFile);
	}
	// 8. Write the padding byte
	fseek(outputFile, paddingPos, SEEK_SET);
	fwrite(&padding, 1, 1, outputFile);

	// 9. Print compression statistics
	fseek(outputFile, 0, SEEK_SET);
	fseek(outputFile, 0, SEEK_END);
	long long compressedFileSize = ftell(outputFile);
	printf("\n---------Compression statistics:------------\n");
	printf("Original File Size: %lld bytes\n", originalFileSize);
	printf("Compressed File Size: %lld bytes\n", compressedFileSize);
	printf("Compression Ratio: %.2f%%\n", (1.0 - (double)compressedFileSize / originalFileSize) * 100);

	// 10. Close files and clean up
	fclose(inputFile);
	fclose(outputFile);
	delete H;
	delete[] inputBuffer;
	delete[] outputBuffer;
	delete ht;
}
void readcodesfromfile(char *filename, char *codes[256])
{
	FILE *inputFile = fopen(filename, "r");
	if (inputFile == NULL)
	{
		printf("Error: Cannot open file!(%s)\n", filename);
		printf("Decompression process cannot continue.\n");
		printf("Exiting the program.\n");
		exit(1);
	}
	char buffer[500];
	int ch;
	while (fscanf(inputFile, "%d: %s\n", &ch, buffer) == 2)
	{
		if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\r')
		{
			buffer[strlen(buffer) - 1] = '\0';
		}
		codes[(unsigned char)ch] = (char *)malloc(strlen(buffer) + 1);
		strcpy(codes[(unsigned char)ch], buffer);
	}
	fclose(inputFile);
}

void decompressFile(const char *inputFilename)
{
	// 1. various variables needed for decompression
	char *codes[256] = {0};							  // to store huffman codes for each character
	char codeFilename[256];							  // to store the name of the codes file
	unsigned char extlen;							  // to store length of original file extension
	char originalExtension[16] = {0};				  // to store original file extension
	unsigned char padding;							  // to store number of padding bits in the last byte
	long long originalFileSize;						  // to store original file size
	long long finalFileSize;						  // to store final decompressed file size
	long long dataSize;								  // to store size of actual data in compressed file
	char outputFilename[256];						  // to store output file name
	unsigned char byte;								  // to read each byte from input file
	long long inputbufferSize = 1024 * 1024 * 4;	  // 1 MB buffer
	long long outputbufferSize = inputbufferSize * 2; // Output buffer can be larger due to decoding
	unsigned char *inputBuffer;						  // to read input file in chunks
	unsigned char *outputBuffer;					  // to store decoded output before writing to file
	long long outIndex = 0;							  // index for output buffer
	// 2. read codes from .cod file
	getfilewithextenstion((char *)inputFilename, codeFilename, (char *)".cod");
	readcodesfromfile(codeFilename, codes);

	// 3. build huffman tree from codes
	HuffmanTree *ht = new HuffmanTree();
	ht->setcodes(codes);
	ht->buildHuffmanTreeFromCodes();
	HuffmanNode *root = ht->root;
	HuffmanNode *current = root;

	// 4. open input file and read metadata
	FILE *inputFile = fopen(inputFilename, "rb");
	if (inputFile == NULL)
	{
		printf("Error: Cannot open input file!%s\n", inputFilename);
		printf("Decompression process cannot continue.\n");
		printf("Exiting the program.\n");
		exit(1);
	}

	fread(&extlen, 1, 1, inputFile);
	if (extlen > 0)
	{
		fread(originalExtension, 1, extlen, inputFile);
		originalExtension[extlen] = '\0';
	}
	fread(&padding, 1, 1, inputFile);
	fseek(inputFile, 0, SEEK_END);
	originalFileSize = ftell(inputFile);
	dataSize = originalFileSize - (1 + extlen + 1);
	fseek(inputFile, 1 + extlen + 1, SEEK_SET); // Move to the position after extension and padding byte

	// 5. create output file
	getfilewithextenstion((char *)inputFilename, outputFilename, originalExtension);
	FILE *outputFile = fopen(outputFilename, "wb");
	if (outputFile == NULL)
	{
		printf("Error: Cannot create output file! %s\n", outputFilename);
		printf("Decompression process cannot continue.\n");
		printf("Exiting the program.\n");
		fclose(inputFile);
		exit(1);
	}
	// 6. read compressed data and decode using Huffman tree
	printf("Decompressing %s into %s\n", inputFilename, outputFilename);
	if (originalFileSize < inputbufferSize)
	{
		inputbufferSize = originalFileSize;
		outputbufferSize = inputbufferSize * 2;
	}
	inputBuffer = new unsigned char[inputbufferSize];
	outputBuffer = new unsigned char[outputbufferSize];
	long long bytesRead;
	long long bytesProcessed = 0;
	while ((bytesRead = fread(inputBuffer, 1, inputbufferSize, inputFile)) > 0)
	{
		for (int j = 0; j < bytesRead; j++)
		{
			bytesProcessed++;
			unsigned char byte = inputBuffer[j];
			int bitsToRead = 8;
			if (bytesProcessed == dataSize)
				bitsToRead = 8 - padding;
			for (int i = 7; i >= 8 - bitsToRead; i--)
			{
				int bit = (byte >> i) & 1;
				if (bit == 0)
				{
					current = current->left;
				}
				else
				{
					current = current->right;
				}
				if (current->left == NULL && current->right == NULL)
				{
					outputBuffer[outIndex++] = current->data;
					if (outIndex == outputbufferSize)
					{
						fwrite(outputBuffer, 1, outputbufferSize, outputFile);
						outIndex = 0;
					}
					current = root;
				}
			}
		}
	}
	// Write any remaining bytes in output buffer
	if (outIndex > 0)
	{
		fwrite(outputBuffer, 1, outIndex, outputFile);
	}
	finalFileSize = ftell(outputFile);

	fclose(inputFile);
	fclose(outputFile);
	// 7. free allocated memory
	delete[] inputBuffer;
	delete[] outputBuffer;
	delete ht;
	for (int i = 0; i < 256; i++)
		free(codes[i]);
	// 8. print decompression statistics
	printf("Decompression completed. Output file: %s\n", outputFilename);
	printf("Original File Size: %lld bytes\n", originalFileSize);
	printf("Decompressed File Size: %lld bytes\n", finalFileSize);
	printf("decompression ratio: %.2f%%\n", (1.0 - (double)originalFileSize / finalFileSize) * 100);
}
int main(int argc, char *argv[])
{
	clock_t startTime = clock();
	if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")))
	{
		printf("Usage: \n");
		printf("  To compress a file: %s -c input_file\n", argv[0]);
		printf("  To decompress a file: %s -d input_file\n", argv[0]);
		return 1;
	}
	if (argc != 3)
	{
		printf("Invalid arguments!\n");
		printf("Use %s --help for usage information.\n", argv[0]);
		return 1;
	}
	if (!strcmp(argv[1], "-c"))
	{
		char ouputfilename[256];
		getfilewithextenstion(argv[2], ouputfilename, (char *)".com");
		printf("Compressing %s into %s...\n", argv[2], ouputfilename);
		compressFile(argv[2], ouputfilename);
		printf("Compression finished.\n");
	}
	else if (!strcmp(argv[1], "-d"))
	{
		decompressFile(argv[2]);
		printf("Decompression finished.\n");
	}
	else if (!strcmp(argv[2], "-c") || !strcmp(argv[2], "-d"))
	{
		printf("Invalid order of arguments!\n");
		printf("Use --help for usage information.\n");
		return 1;
	}
	else
	{
		printf("Invalid option! Use -c for compression or -d for decompression.\n");
		printf("Use --help for usage information.\n");
		return 1;
	}
	clock_t endTime = clock();
	printf("Time taken: %.2f seconds\n", double(endTime - startTime) / CLOCKS_PER_SEC);
	return 0;
}
