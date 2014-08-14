#ifndef IO_H_H_H //just to be safe
#define IO_H_H_H

#include <stdint.h>
#include <cstdlib>

class IO{
public:
	//excludes EOF from buffer
	static uint8_t* readWholeFile(const char* fileName, long* sizeOut, const char* openFlags = "rb"){
		FILE* f = fopen(fileName, openFlags);
		uint8_t* buffer = NULL;
		if (sizeOut) *sizeOut = 0;

		if (f){
			fseek(f, 0, SEEK_SET);
			long start = ftell(f);

			fseek(f, 0, SEEK_END);
			long size = ftell(f) - start;
			
			printf("opened file %s with size %ld\n", fileName, size);
	
			fseek(f, 0, SEEK_SET);

			buffer = (uint8_t*)malloc(size + 1);
			if (fread(buffer, 1, size, f) != size){
				free(buffer); //all or nothing
				buffer = NULL;
				size = 0;
			}
			
			else{
				buffer[size] = 0; //null terminator for strings
			}
			
			if (sizeOut) *sizeOut = size;
			fclose(f);
		}
		else{
			fprintf(stderr, "could not open file %s for reading (flags %s)\n", fileName, openFlags);
			perror("reason");
		}
		
		return buffer;
	}
};

#endif
