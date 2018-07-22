
hello.so.o:     file format elf64-x86-64


Disassembly of section .init:

0000000000000490 <_init>:
 490:	48 83 ec 08          	sub    $0x8,%rsp
 494:	e8 47 00 00 00       	callq  4e0 <call_gmon_start>
 499:	e8 e2 00 00 00       	callq  580 <frame_dummy>
 49e:	e8 2d 01 00 00       	callq  5d0 <__do_global_ctors_aux>
 4a3:	48 83 c4 08          	add    $0x8,%rsp
 4a7:	c3                   	retq   

Disassembly of section .plt:

00000000000004b0 <puts@plt-0x10>:
 4b0:	ff 35 3a 0b 20 00    	pushq  0x200b3a(%rip)        # 200ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
 4b6:	ff 25 3c 0b 20 00    	jmpq   *0x200b3c(%rip)        # 200ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
 4bc:	0f 1f 40 00          	nopl   0x0(%rax)

00000000000004c0 <puts@plt>:
 4c0:	ff 25 3a 0b 20 00    	jmpq   *0x200b3a(%rip)        # 201000 <_GLOBAL_OFFSET_TABLE_+0x18>
 4c6:	68 00 00 00 00       	pushq  $0x0
 4cb:	e9 e0 ff ff ff       	jmpq   4b0 <_init+0x20>

00000000000004d0 <__cxa_finalize@plt>:
 4d0:	ff 25 32 0b 20 00    	jmpq   *0x200b32(%rip)        # 201008 <_GLOBAL_OFFSET_TABLE_+0x20>
 4d6:	68 01 00 00 00       	pushq  $0x1
 4db:	e9 d0 ff ff ff       	jmpq   4b0 <_init+0x20>

Disassembly of section .text:

00000000000004e0 <call_gmon_start>:
 4e0:	48 83 ec 08          	sub    $0x8,%rsp
 4e4:	48 8b 05 e5 0a 20 00 	mov    0x200ae5(%rip),%rax        # 200fd0 <_DYNAMIC+0x180>
 4eb:	48 85 c0             	test   %rax,%rax
 4ee:	74 02                	je     4f2 <call_gmon_start+0x12>
 4f0:	ff d0                	callq  *%rax
 4f2:	48 83 c4 08          	add    $0x8,%rsp
 4f6:	c3                   	retq   
 4f7:	90                   	nop
 4f8:	90                   	nop
 4f9:	90                   	nop
 4fa:	90                   	nop
 4fb:	90                   	nop
 4fc:	90                   	nop
 4fd:	90                   	nop
 4fe:	90                   	nop
 4ff:	90                   	nop

0000000000000500 <__do_global_dtors_aux>:
 500:	55                   	push   %rbp
 501:	80 3d 10 0b 20 00 00 	cmpb   $0x0,0x200b10(%rip)        # 201018 <__bss_start>
 508:	48 89 e5             	mov    %rsp,%rbp
 50b:	41 54                	push   %r12
 50d:	53                   	push   %rbx
 50e:	75 62                	jne    572 <__do_global_dtors_aux+0x72>
 510:	48 83 3d c8 0a 20 00 	cmpq   $0x0,0x200ac8(%rip)        # 200fe0 <_DYNAMIC+0x190>
 517:	00 
 518:	74 0c                	je     526 <__do_global_dtors_aux+0x26>
 51a:	48 8b 3d ef 0a 20 00 	mov    0x200aef(%rip),%rdi        # 201010 <__dso_handle>
 521:	e8 aa ff ff ff       	callq  4d0 <__cxa_finalize@plt>
 526:	48 8d 1d 13 09 20 00 	lea    0x200913(%rip),%rbx        # 200e40 <__DTOR_END__>
 52d:	4c 8d 25 04 09 20 00 	lea    0x200904(%rip),%r12        # 200e38 <__DTOR_LIST__>
 534:	48 8b 05 e5 0a 20 00 	mov    0x200ae5(%rip),%rax        # 201020 <dtor_idx.6533>
 53b:	4c 29 e3             	sub    %r12,%rbx
 53e:	48 c1 fb 03          	sar    $0x3,%rbx
 542:	48 83 eb 01          	sub    $0x1,%rbx
 546:	48 39 d8             	cmp    %rbx,%rax
 549:	73 20                	jae    56b <__do_global_dtors_aux+0x6b>
 54b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
 550:	48 83 c0 01          	add    $0x1,%rax
 554:	48 89 05 c5 0a 20 00 	mov    %rax,0x200ac5(%rip)        # 201020 <dtor_idx.6533>
 55b:	41 ff 14 c4          	callq  *(%r12,%rax,8)
 55f:	48 8b 05 ba 0a 20 00 	mov    0x200aba(%rip),%rax        # 201020 <dtor_idx.6533>
 566:	48 39 d8             	cmp    %rbx,%rax
 569:	72 e5                	jb     550 <__do_global_dtors_aux+0x50>
 56b:	c6 05 a6 0a 20 00 01 	movb   $0x1,0x200aa6(%rip)        # 201018 <__bss_start>
 572:	5b                   	pop    %rbx
 573:	41 5c                	pop    %r12
 575:	5d                   	pop    %rbp
 576:	c3                   	retq   
 577:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
 57e:	00 00 

0000000000000580 <frame_dummy>:
 580:	48 83 3d c0 08 20 00 	cmpq   $0x0,0x2008c0(%rip)        # 200e48 <__JCR_END__>
 587:	00 
 588:	55                   	push   %rbp
 589:	48 89 e5             	mov    %rsp,%rbp
 58c:	74 1a                	je     5a8 <frame_dummy+0x28>
 58e:	48 8b 05 43 0a 20 00 	mov    0x200a43(%rip),%rax        # 200fd8 <_DYNAMIC+0x188>
 595:	48 85 c0             	test   %rax,%rax
 598:	74 0e                	je     5a8 <frame_dummy+0x28>
 59a:	5d                   	pop    %rbp
 59b:	48 8d 3d a6 08 20 00 	lea    0x2008a6(%rip),%rdi        # 200e48 <__JCR_END__>
 5a2:	ff e0                	jmpq   *%rax
 5a4:	0f 1f 40 00          	nopl   0x0(%rax)
 5a8:	5d                   	pop    %rbp
 5a9:	c3                   	retq   
 5aa:	90                   	nop
 5ab:	90                   	nop

00000000000005ac <main>:
 5ac:	55                   	push   %rbp
 5ad:	48 89 e5             	mov    %rsp,%rbp
 5b0:	48 8d 3d 5f 00 00 00 	lea    0x5f(%rip),%rdi        # 616 <_fini+0xe>
 5b7:	e8 04 ff ff ff       	callq  4c0 <puts@plt>
 5bc:	b8 00 00 00 00       	mov    $0x0,%eax
 5c1:	5d                   	pop    %rbp
 5c2:	c3                   	retq   
 5c3:	90                   	nop
 5c4:	90                   	nop
 5c5:	90                   	nop
 5c6:	90                   	nop
 5c7:	90                   	nop
 5c8:	90                   	nop
 5c9:	90                   	nop
 5ca:	90                   	nop
 5cb:	90                   	nop
 5cc:	90                   	nop
 5cd:	90                   	nop
 5ce:	90                   	nop
 5cf:	90                   	nop

00000000000005d0 <__do_global_ctors_aux>:
 5d0:	55                   	push   %rbp
 5d1:	48 89 e5             	mov    %rsp,%rbp
 5d4:	53                   	push   %rbx
 5d5:	48 83 ec 08          	sub    $0x8,%rsp
 5d9:	48 8b 05 48 08 20 00 	mov    0x200848(%rip),%rax        # 200e28 <__CTOR_LIST__>
 5e0:	48 83 f8 ff          	cmp    $0xffffffffffffffff,%rax
 5e4:	74 19                	je     5ff <__do_global_ctors_aux+0x2f>
 5e6:	48 8d 1d 3b 08 20 00 	lea    0x20083b(%rip),%rbx        # 200e28 <__CTOR_LIST__>
 5ed:	0f 1f 00             	nopl   (%rax)
 5f0:	48 83 eb 08          	sub    $0x8,%rbx
 5f4:	ff d0                	callq  *%rax
 5f6:	48 8b 03             	mov    (%rbx),%rax
 5f9:	48 83 f8 ff          	cmp    $0xffffffffffffffff,%rax
 5fd:	75 f1                	jne    5f0 <__do_global_ctors_aux+0x20>
 5ff:	48 83 c4 08          	add    $0x8,%rsp
 603:	5b                   	pop    %rbx
 604:	5d                   	pop    %rbp
 605:	c3                   	retq   
 606:	90                   	nop
 607:	90                   	nop

Disassembly of section .fini:

0000000000000608 <_fini>:
 608:	48 83 ec 08          	sub    $0x8,%rsp
 60c:	e8 ef fe ff ff       	callq  500 <__do_global_dtors_aux>
 611:	48 83 c4 08          	add    $0x8,%rsp
 615:	c3                   	retq   
