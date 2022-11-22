#include <inc/lib.h>

// malloc()
//	This function use FIRST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
uint32 arr[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];
uint32 size_arr[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];
uint32 empty_arr[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];
uint32 size_empty_arr[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];
int cnt_arr = 0, cnt_empty_arr = 0;
void* malloc(uint32 size) {
	//TODO: [PROJECT 2021 - [2] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	int size_m = size;
	size = ROUNDUP(size, PAGE_SIZE);

	if (!cnt_arr && (size < USER_HEAP_MAX - USER_HEAP_START)) {
		arr[cnt_arr] = USER_HEAP_START;
		size_arr[cnt_arr] = size;
		empty_arr[cnt_empty_arr] = USER_HEAP_START + size;
		size_empty_arr[cnt_empty_arr] = USER_HEAP_MAX
				- empty_arr[cnt_empty_arr];
		cnt_empty_arr++;
		cnt_arr++;
		sys_allocateMem(arr[cnt_arr - 1], size_m);

		return (void*) arr[cnt_arr - 1];
	}
	else {
		uint32 va = USER_HEAP_MAX;
		uint32 Min_Size = USER_HEAP_MAX - USER_HEAP_START;
		int ind = -1;

		// best fit strategy
		for (int i = 0; i < cnt_empty_arr; ++i) {
			if (size_empty_arr[i] < Min_Size && size_empty_arr[i] >= size
					&& size_empty_arr[i] > 0 && empty_arr[i] > 0) {
				va = empty_arr[i];
				Min_Size = size_empty_arr[i];
				ind = i;
			} else if (size_empty_arr[i] == Min_Size
					&& size_empty_arr[i] >= size && size_empty_arr[i] > 0
					&& empty_arr[i] > 0) {
				if (empty_arr[i] < va) {
					va = empty_arr[i];
					Min_Size = size_empty_arr[i];
					ind = i;
				}
			}
		}

		if (ind == -1 || va >= USER_HEAP_MAX)
			return NULL;

		else {
			arr[cnt_arr] = empty_arr[ind];
			size_arr[cnt_arr] = size;
			empty_arr[ind] += size;
			size_empty_arr[ind] -= size;
			cnt_arr++;
			sys_allocateMem(arr[cnt_arr - 1], size_m);
			return (void*) arr[cnt_arr - 1];
		}
	}
	//This function should find the space of the required range
	//using the BEST FIT strategy
	//refer to the project presentation and documentation for details

	return NULL;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address) {
	//TODO: [PROJECT 2021 - [2] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	uint32 va = (uint32) virtual_address;
	int i, found_in_arr = 0;
	int size_va = 0;
	uint32 ret_size = 0, ret_va = (uint32) virtual_address;
	for (i = 0; i < cnt_arr; i++) {
		if (va == arr[i]) {
			found_in_arr = 1;
			ret_size = size_arr[i];
			break;
		}
	}

	if (found_in_arr == 1) {
		size_va = size_arr[i];
		arr[i] = 0;
		size_arr[i] = 0;
		int found = 0;
		// if it is between two free mem
		for (int i = 0; i < cnt_empty_arr; ++i) {
			if (empty_arr[i] > va && empty_arr[i] > 0
					&& empty_arr[i] == va + size_va) {
				for (int j = 0; j < cnt_empty_arr; ++j) {
					if (empty_arr[j] < va && empty_arr[j] > 0
							&& empty_arr[j] + size_empty_arr[j] == va) {
						found = 1;
						size_va += size_empty_arr[i];
						empty_arr[i] = 0;
						size_empty_arr[i] = 0;
						size_empty_arr[j] += size_va;
						break;
					}
				}
				if (found == 1)
					break;
			}
		}

		// if it is after free mem
		if (found == 0) {
			for (int i = 0; i < cnt_empty_arr; ++i) {
				if (empty_arr[i] < va && empty_arr[i] > 0
						&& empty_arr[i] + size_empty_arr[i] == va) {
					found = 1;
					size_empty_arr[i] += size_va;
					break;
				}
			}

			// if it is before free mem
			for (int i = 0; i < cnt_empty_arr; ++i) {
				if (empty_arr[i] > va && empty_arr[i] > 0
						&& empty_arr[i] == va + size_va) {
					found = 1;
					empty_arr[cnt_empty_arr] = va;
					size_empty_arr[cnt_empty_arr] = size_va + size_empty_arr[i];
					cnt_empty_arr++;
					empty_arr[i] = 0;
					size_empty_arr[i] = 0;
					break;
				}
			}
		}

		// if it between two allocated mem
		if (found == 0) {
			empty_arr[cnt_empty_arr] = va;
			size_empty_arr[cnt_empty_arr] = size_va;
			cnt_empty_arr++;
		}

		sys_freeMem(ret_va, ret_size);
	}

	//you should get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
}

//==================================================================================//
//================================ OTHER FUNCTIONS =================================//
//==================================================================================//

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable) {
	panic("this function is not required...!!");
	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName) {
	panic("this function is not required...!!");
	return 0;
}

void sfree(void* virtual_address) {
	panic("this function is not required...!!");
}

void *realloc(void *virtual_address, uint32 new_size) {
	panic("this function is not required...!!");
	return 0;
}

void expand(uint32 newSize) {
	panic("this function is not required...!!");
}
void shrink(uint32 newSize) {
	panic("this function is not required...!!");
}

void freeHeap(void* virtual_address) {
	panic("this function is not required...!!");
}
