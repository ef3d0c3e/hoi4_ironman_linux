#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>

typedef struct
{
	const char* sig;
	const char* mask;
	size_t len;
} signature;

// checksum_gate+0x3f:
// 85 c0     test eax, eax
// 0f 84 c3  sete bl
// e8        call [...]
static const signature sig_checksum_gate = { .sig = "\x85\xC0\x0F\x94\x00\xE8",
                                             .mask = "xxxx?x",
                                             .len = 6 };

static uintptr_t
find_sig(uintptr_t start, uintptr_t end, const signature* sig)
{
	assert(start <= end);
	const unsigned char* mem = (const unsigned char*)start;

	for (size_t i = 0; start + i + sig->len < end; ++i) {
		for (size_t j = 0; j < sig->len; ++j) {
			const unsigned char b = sig->mask[j] == '?' ? mem[i + j] : (unsigned char)sig->sig[j];
			if (mem[i + j] != b)
				break;
			if (j + 1 == sig->len)
				return start + i;
		}
	}
	return 0;
}

static int
find_module(const char* module_name, uintptr_t* start, uintptr_t* end)
{
	const size_t namelen = strlen(module_name);
	*start = *end = 0;

	FILE* file = fopen("/proc/self/maps", "r");
	assert(file && "Failed to open file");
	char* line = NULL;
	size_t len = 0;
	ssize_t nread;
	while ((nread = getline(&line, &len, file)) != -1) {
		if ((size_t)nread < namelen + 1)
			continue;
		if (memcmp(line + nread - namelen - 1, module_name, namelen)) {
			if (*start)
				break;
			continue;
		}

		uintptr_t dummy;
		const int __attribute__((unused)) result =
		  sscanf(line, "%lx-%lx", *start ? &dummy : start, end);
		assert(result == 2);
	}
	fclose(file);
	free(line);
	return *start != 0;
}

static void
set_prot(uintptr_t start, uintptr_t end, int prot)
{
	const uintptr_t page = (uintptr_t)sysconf(_SC_PAGESIZE);

	start &= ~(page - 1);
	for (size_t i = 0; start + i < end; i += page) {
		if (mprotect((void*)(start + i), page, prot))
		{
			fprintf(stderr, "Failed to mprotect %#lx: %m\n", start + i);
			exit(1);
		}
	}
}

void __attribute__((constructor))
startup()
{
	uintptr_t start, end;
	if (!find_module("/hoi4", &start, &end)) {
		fprintf(stderr, "Failed to find hoi4 maps\n");
		exit(1);
	}

	const uintptr_t result = find_sig(start, end, &sig_checksum_gate);
	if (!result) {
		fprintf(stderr, "Failed to find checksum_gate\n");
		exit(1);
	}

	set_prot(result, result+1, PROT_READ | PROT_WRITE);
	// test -> xor
	*(unsigned char*)result = 0x31;
	set_prot(result, result+1, PROT_READ | PROT_EXEC);
}
