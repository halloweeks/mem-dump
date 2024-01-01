#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>

#define CHUNK_SIZE 65536

#ifndef process_vm_readv
	#include <sys/syscall.h>
	#include <asm/unistd.h>
	
	ssize_t process_vm_readv(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt, unsigned long flags) {
		return syscall(__NR_process_vm_readv, pid, local_iov, liovcnt, remote_iov, riovcnt, flags);
	}
#endif

ssize_t read_process_memory(pid_t pid, uintptr_t address, void *value, size_t size) {
	struct iovec local[1];
	struct iovec remote[1];
	local[0].iov_base = value;
	local[0].iov_len = size;
	remote[0].iov_base = (void*)address;
	remote[0].iov_len = size;
	return process_vm_readv(pid, local, 1, remote, 1, 0);
}

pid_t find_pid(const char *process_name) {
	DIR *dir = opendir("/proc");
	struct dirent *entry = NULL;
	char cmdline_path[256];
	char cmdline[256];
	int fd;
	
	if (dir == NULL) {
		return -1;
	}
	
	while ((entry = readdir(dir)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) || (entry->d_type != DT_DIR) || (strspn(entry->d_name, "0123456789") != strlen(entry->d_name))) {
			continue;
		}
		snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);
		fd = open(cmdline_path, O_RDONLY);
		read(fd, cmdline, 256);
		close(fd);
		if (strncmp(cmdline, process_name, strlen(process_name)) == 0) {
			closedir(dir);
			return atoi(entry->d_name);
		}
	}
	closedir(dir);
	return -1;
}

uint8_t get_module_address(pid_t process_id, const char *module_name, unsigned long long *start_addr, unsigned long long *end_addr) {
	char filename[256];
	char line[1024];
	FILE *fp = NULL;
	uint8_t address_found = 0;
	unsigned long long start, end;
	
	snprintf(filename, sizeof(filename), "/proc/%d/maps", process_id);
	
	if (!(fp = fopen(filename, "r"))) {
		return 0;
	}
	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, module_name)) {
			if (sscanf(line, "%llx-%llx", &start, &end) == 2) {
				address_found = 1;
				*start_addr = start;
				*end_addr = end;
				break;
			}
		}
	}
	fclose(fp);
	return address_found;
}

void convertBytes(long long bytes, char result[50]) {
    if (bytes < 0) {
        sprintf(result, "Invalid input: Negative bytes");
        return;
    }

    if (bytes < 1024) {
        sprintf(result, "%lld bytes", bytes);
    } else if (bytes < 1024 * 1024) {
        sprintf(result, "%.2f KB", (double)bytes / 1024);
    } else if (bytes < 1024 * 1024 * 1024) {
        sprintf(result, "%.2f MB", (double)bytes / (1024 * 1024));
    } else {
        sprintf(result, "%.2f GB", (double)bytes / (1024 * 1024 * 1024));
    }
}

int main(int argc, const char *argv[]) {
	const char* package = "com.tencent.ig"; // Process Name
	const char* module = "libUE4.so"; // Module name
	unsigned long long start = 0, end = 0; // module start and end memory address
	char result[50];
	
	pid_t pid = find_pid(package);
	
	if (pid == -1) {
		printf("Failed to open %s process\n", package);
		return 1;
	}
	
	if (!get_module_address(pid, module, &start, &end)) {
		printf("Failed to get %s base module address from %s package\n", module, package);
		return 1;
	}
	
	convertBytes(end - start, result);
	
	printf("%s Module Size: %s\n", module, result);
	
	uint8_t chunk[CHUNK_SIZE];
	unsigned long long remaining_size = end - start;
    ssize_t bytesRead, bytesWritten;
	unsigned long long address = start;
	
	snprintf(result, sizeof(result), "libue4_dump_%d.bin", pid);
	
	int fout = open(result, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	if (fout == -1) {
		printf("Failed to create %s file\n", result);
		return 1;
	}
	
	while (remaining_size > 0) {
		size_t bytesToRead = remaining_size < CHUNK_SIZE ? remaining_size : CHUNK_SIZE;
		
		if (read_process_memory(pid, address, chunk, bytesToRead) != bytesToRead) {
			printf("Unable to read memory at %p\n", (void*)&address);
			break;
		}
		
		if (write(fout, chunk, bytesToRead) != bytesToRead) {
			printf("Failed to write %s\n", result);
			return 1;
		}
		
		remaining_size -= bytesToRead;
		address += bytesToRead;
	}
	
	close(fout);
	return 0;
}