#include "9cc.h"

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  static int s_lavel_no = 1;
  int lavel_no;
  Node *current;

  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;  
  case ND_IF:
    lavel_no = s_lavel_no;
    s_lavel_no++;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  Lelse%d\n", lavel_no);
    gen(node->rhs);
    printf("  jmp Lend%d\n", lavel_no);
    printf("Lelse%d:\n", lavel_no);
    if (node->els)
      gen(node->els);
    else
      printf("  push rax\n");
    printf("Lend%d:\n", lavel_no);
    return;
  case ND_WHILE:
    lavel_no = s_lavel_no;
    s_lavel_no++;
    printf("Lbegin%d:\n", lavel_no);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  Lend%d\n", lavel_no);
    gen(node->rhs);
    printf("  jmp Lbegin%d\n", lavel_no);
    printf("Lend%d:\n", lavel_no);
    return;
  case ND_FOR:
    lavel_no = s_lavel_no;
    s_lavel_no++;
    gen(node->for_clause1);
    printf("Lbegin%d:\n", lavel_no);
    gen(node->for_expression2);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  Lend%d\n", lavel_no);
    gen(node->rhs);
    gen(node->for_expression3);
    printf("  jmp Lbegin%d\n", lavel_no);
    printf("Lend%d:\n", lavel_no);
    return;
  case ND_BLOCK:
    current = node->lhs;
    while (current) {
      gen(current);
      printf("  pop rax\n");
      current = current->next;
    }
    printf("  push rax\n");
    return;
  case ND_FN: {
    char *registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int i;
    int len = sizeof registers / sizeof registers[0];
    current = node->lhs;
    for (i = 0; i < len && current; i++) {
      gen(current);
      current = current->next;
    }
    current = node->lhs;
    for (; 0 < i ; i--) {
      printf("  pop %s\n", registers[i - 1]);
      current = current->next;
    }
    printf("  push r15\n");
    printf("  mov r15, rsp\n");
    printf("  and spl, 0xF0\n");
    printf("  call %1$.*2$s\n", node->name, node->len);
    printf("  mov rsp, r15\n");
    printf("  pop r15\n");
    printf("  push rax\n");
    return;
  }
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  
  printf("  push rax\n");
}
