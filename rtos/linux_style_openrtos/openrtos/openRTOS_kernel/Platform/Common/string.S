	.code32
	.text

	.global	memcmp
memcmp:
	mov	%edi,%eax		// Save EDI
	mov	%esi,%edx		// Save ESI
	mov	4(%esp),%edi		// s1 -> EDI
	mov	8(%esp),%esi		// s2 -> ESI
	mov	12(%esp),%ecx		// Number of bytes to compare -> ECX
	cld				// Pointer auto-increment
	or	%ecx,%ecx		// Set ZF flag just in case ECX = 0 and repz is not executed
	repz cmpsb			// Compare bytes
	xchg	%eax,%edi		// Restore EDI
	xchg	%edx,%esi		// Restore ESI
	xchg	%ecx,%eax		// If strings are equal, ECX = 0, move it to EAX
	jz	1f			// If srings are equal, exit
	movzbl	-1(%ecx),%eax		// First non-equal byte of s1 -> EAX
	movzbl	-1(%edx),%edx		// First non-equal byte of s2 -> EDX
	sub	%edx,%eax		// Calculate return value (<0 or >0)
1:
	ret


	.global	memcpy
memcpy:
	mov	%edi,%eax		// Save EDI
	mov	%esi,%edx		// Save ESI
	mov	4(%esp),%edi		// s1 -> EDI
	mov	8(%esp),%esi		// s2 -> ESI
	mov	12(%esp),%ecx		// Number of bytes to copy -> ECX
	cld				// Pointer auto-increment
	rep movsb			// Copy bytes
	mov	%eax,%edi		// Restore EDI
	mov	%edx,%esi		// Restore ESI
	mov	4(%esp),%eax		// Return s1
	ret


	.global	strcmp
	.global	strncmp
strcmp:
	xor	%ecx,%ecx		// Compare up to 4GB
	jmp	1f
strncmp:
	mov	12(%esp),%ecx		// Maximal number of bytes to compare -> ECX
	jecxz	4f			// Nothing to do if it's zero
1:
	mov	%edi,%eax		// Save EDI
	mov	%esi,%edx		// Save ESI
	mov	4(%esp),%edi		// s1 -> EDI
	mov	8(%esp),%esi		// s2 -> ESI
	cld				// Pointer auto-increment
2:
	cmpsb				// Compare one byte
	jne	3f			// Exit if not equal
	cmpb	$0,-1(%esi)		// Check is the byte was zero (end of string)
	loopnz	2b			// Repeat if not and byte counter is not zero
3:
	xchg	%eax,%edi		// Restore EDI, end of s1 comparison -> EAX
	xchg	%edx,%esi		// Restore ESI, end of s2 comparison -> EDX
	jz	4f			// Strings are equal
	movzbl	-1(%eax),%eax		// First non-equal byte from s1 -> EAX
	movzbl	-1(%edx),%edx		// First non-equal byte from s2 -> EDX
	sub	%edx,%eax		// Calculate return value (<0 or >0)
	ret
4:
	xor	%eax,%eax		// Return zero
	ret


	.global	strcpy
	.global	strncpy
strcpy:
	xor	%ecx,%ecx		// Copy up to 4GB
	jmp	1f
strncpy:
	mov	12(%esp),%ecx		// Maximal number of bytes to copy
	jecxz	4f			// Nothing to do if it's zero
1:
	mov	%esi,%eax		// Save ESI
	mov	%edi,%edx		// Save EDI
	mov	4(%esp),%edi		// s1 -> EDI
	mov	8(%esp),%esi		// s2 -> ESI
	cld				// Pointer auto-increment
2:
	movsb				// Copy one byte
	cmpb	$0,-1(%esi)		// Check if the copied byte was zero
	loopnz	2b			// Repeat if not and byte count is not zero
	mov	%eax,%esi		// Restore ESI
	or	%ecx,%ecx		// Check ECX. We never copy more than 2GB, so
	js	3f			// negative ECX means that we're in strcpy()
	xor	%al,%al			// Zero AL
	rep stosb			// Pad destination with zeros until byte count is zero
3:
	mov	%edx,%edi		// Restore EDI
4:
	mov	4(%esp),%eax		// Return s1
	ret


	.global	strchr
strchr:
	mov	%esi,%edx		// Save ESI
	mov	4(%esp),%esi		// String pointer -> ESI
	mov	8(%esp),%ecx		// Char to find -> ECX
	cld				// Pointer auto-increment
1:
	lodsb				// Next char from the string -> AL
	cmp	%al,%cl			// Found?
	je	2f			// Exit if found
	or	%al,%al			// Check if zero (end of string) reached
	jnz	1b			// Go to next char if not
	xor	%eax,%eax		// Char not found in the string, return NULL
	jmp	3f
2:
	lea	-1(%esi),%eax		// Pointer to the found char -> EAX
3:
	mov	%edx,%esi		// Restore ESI
	ret


	.global	strlen
	.global	strnlen
strlen:
	mov	$-1,%ecx		// Examine up to 4GB
	jmp	1f
strnlen:
	mov	8(%esp),%ecx		// maxlen -> ECX
1:
	xor	%eax,%eax
	jecxz	3f			// Exit if maxlen = 0
	mov	%edi,%edx		// Save EDI
	mov	4(%esp),%edi		// String pointer -> EDI
	cld				// Pointer auto-increment
	repnz scasb			// Scan the string for AL (which is zero here)
	jnz	2f			// Zero not found, maxlen reached
	dec	%edi			// Zero found, not counted in string length
2:
	mov	%edi,%eax
	sub	4(%esp),%eax		// Calculate string length
	mov	%edx,%edi		// Restore EDI
3:
	ret


	.global	strcat
	.global	strncat
strcat:
	mov	$-1,%ecx		// Append any number of bytes until zero
	jmp	1f
strncat:
	mov	12(%esp),%ecx		// Append no more than this number of bytes
	jecxz	5f			// Nothing to do if it's zero
1:
	mov	%edi,%edx		// Save EDI
	mov	4(%esp),%edi		// s1 -> EDI
	xor	%al,%al			// Look for zero byte (end of string)
	cld				// with pointer auto-increment
2:
	scasb				// Scan
	jnz	2b
	dec	%edi			// EDI points to the zero byte
	mov	%esi,%eax		// Save ESI
	mov	8(%esp),%esi		// s2 -> ESI
3:
	movsb				// Copy one byte
	cmpb	$0,-1(%esi)		// Check if the copied byte was zero
	loopnz	3b			// Repeat if not and byte count is not zero
	jz	4f			// If zero byte was copied, we're done
	movb	$0,(%edi)		// Else append zero byte to resulting string
4:
	mov	%eax,%esi		// Restore ESI
	mov	%edx,%edi		// Restore EDI
5:
	mov	4(%esp),%eax		// Return s1
	ret

	.end
