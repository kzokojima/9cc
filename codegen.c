#include "9cc.h"

void emit(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  emit("# var:%1$.*2$s", node->name, node->len);
  emit("  mov rax, rbp");
  emit("  sub rax, %d", node->offset);
  emit("  push rax");
}

void gen(Node *node) {
  static int s_lavel_no = 1;
  int lavel_no;
  Node *current;
  char *registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  char *registers_32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
  int registers_len = sizeof registers / sizeof registers[0];
  int i;

  switch (node->kind) {
  case ND_NUM:
    emit("  push %d", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    emit("  pop rax");
    if (node->type->ty == ARRAY) {
        // do nothing
    } else {
      if (get_type_size(node->type->ty) == 4)
        emit("  mov eax, [rax]");
      else
        emit("  mov rax, [rax]");
    }
    emit("  push rax");
    return;
  case ND_ASSIGN:
    if (node->lhs->kind == ND_DEREF) {
      if (node->lhs->lhs->kind == ND_LVAR) {
        if (node->lhs->lhs->type->ty == ARRAY) {
          // *array = ...;
          gen_lval(node->lhs->lhs);
        } else {
          // *pointer = ...;
          gen(node->lhs->lhs);
        }
      } else if (node->lhs->lhs->kind == ND_ADD) {
        // *(array + 1) = ...;
        gen_lval(node->lhs->lhs->lhs);
        gen(node->lhs->lhs->rhs);
        emit("  pop rdi");
        emit("  pop rax");
        emit("  imul rdi, %d", get_type_size(node->lhs->lhs->lhs->type->ptr_to->ty));
        emit("  add rax, rdi");
        emit("  push rax");
      } else {
        error("代入式が不正です");
      }
    } else {
      gen_lval(node->lhs);
    }
    gen(node->rhs);
    emit("  pop rdi");
    emit("  pop rax");
    if (node->lhs->kind == ND_DEREF) {
      if (node->lhs->lhs->kind == ND_LVAR) {
        if (get_type_size(node->lhs->lhs->type->ty) == 8)
          emit("  mov [rax], rdi");
        else
          emit("  mov [rax], edi");
      } else if (node->lhs->lhs->kind == ND_ADD) {
        if (get_type_size(node->lhs->lhs->lhs->type->ptr_to->ty) == 8)
          emit("  mov [rax], rdi");
        else
          emit("  mov [rax], edi");
      }
    } else {
      if (get_type_size(node->lhs->type->ty) == 8)
        emit("  mov [rax], rdi");
      else
        emit("  mov [rax], edi");
    }
    emit("  push rdi");
    return;
  case ND_RETURN:
    gen(node->lhs);
    emit("  pop rax");
    emit("  mov rsp, rbp");
    emit("  pop rbp");
    emit("  ret");
    return;
  case ND_IF:
    lavel_no = s_lavel_no;
    s_lavel_no++;
    gen(node->lhs);
    emit("  pop rax");
    emit("  cmp rax, 0");
    emit("  je  Lelse%d", lavel_no);
    gen(node->rhs);
    emit("  jmp Lend%d", lavel_no);
    emit("Lelse%d:", lavel_no);
    if (node->els)
      gen(node->els);
    else
      emit("  push rax");
    emit("Lend%d:", lavel_no);
    return;
  case ND_WHILE:
    lavel_no = s_lavel_no;
    s_lavel_no++;
    emit("Lbegin%d:", lavel_no);
    gen(node->lhs);
    emit("  pop rax");
    emit("  cmp rax, 0");
    emit("  je  Lend%d", lavel_no);
    gen(node->rhs);
    emit("  jmp Lbegin%d", lavel_no);
    emit("Lend%d:", lavel_no);
    return;
  case ND_FOR:
    lavel_no = s_lavel_no;
    s_lavel_no++;
    gen(node->for_clause1);
    emit("Lbegin%d:", lavel_no);
    gen(node->for_expression2);
    emit("  pop rax");
    emit("  cmp rax, 0");
    emit("  je  Lend%d", lavel_no);
    gen(node->rhs);
    gen(node->for_expression3);
    emit("  jmp Lbegin%d", lavel_no);
    emit("Lend%d:", lavel_no);
    return;
  case ND_BLOCK:
    current = node->lhs;
    while (current) {
      gen(current);
      emit("  pop rax");
      current = current->next;
    }
    emit("  push rax");
    return;
  case ND_FN: {
    current = node->lhs;
    for (i = 0; i < registers_len && current; i++) {
      gen(current);
      current = current->next;
    }
    current = node->lhs;
    for (; 0 < i ; i--) {
      emit("  pop %s", registers[i - 1]);
      current = current->next;
    }
    emit("  push r15");
    emit("  mov r15, rsp");
    emit("  and spl, 0xF0");
    emit("  call %1$.*2$s", node->name, node->len);
    emit("  mov rsp, r15");
    emit("  pop r15");
    emit("  push rax");
    return;
  }
  case ND_FN_DEF:
    emit(".global %1$.*2$s", node->name, node->len);
    emit("%1$.*2$s:", node->name, node->len);

    // プロローグ
    // 変数の領域を確保する
    emit("  push rbp");
    emit("  mov rbp, rsp");
    if (node->lvar_size) {
      emit("  sub rsp, %d", node->lvar_size);
    }

    // パラメータ
    current = node->lhs;
    for (i = 0; i < registers_len && current; i++) {
      if (get_type_size(current->type->ty) == 8)
        emit("  mov [rbp - %d], %s", current->offset, registers[i]);
      else
        emit("  mov [rbp - %d], %s", current->offset, registers_32[i]);
      current = current->next;
    }

    gen(node->rhs);

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    emit("  mov rsp, rbp");
    emit("  pop rbp");
    emit("  ret");
    return;
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    emit("  pop rax");
    emit("  mov rax, [rax]");
    emit("  push rax");
    return;
  case ND_DEFVAR:
    emit("  push rax");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  emit("  pop rdi");
  emit("  pop rax");

  switch (node->kind) {
  case ND_ADD:
    if (node->lhs->kind == ND_LVAR && (node->lhs->type->ty == ARRAY || node->lhs->type->ty == PTR)) {
      emit("  imul rdi, %d", get_type_size(node->lhs->type->ptr_to->ty));
    } else if (node->rhs->kind == ND_LVAR && (node->rhs->type->ty == ARRAY || node->rhs->type->ty == PTR)) {
      emit("  imul rax, %d", get_type_size(node->rhs->type->ptr_to->ty));
    }
    emit("  add rax, rdi");
    break;
  case ND_SUB:
    emit("  sub rax, rdi");
    break;
  case ND_MUL:
    emit("  imul rax, rdi");
    break;
  case ND_DIV:
    emit("  cqo");
    emit("  idiv rdi");
    break;
  case ND_EQ:
    emit("  cmp rax, rdi");
    emit("  sete al");
    emit("  movzb rax, al");
    break;
  case ND_NE:
    emit("  cmp rax, rdi");
    emit("  setne al");
    emit("  movzb rax, al");
    break;
  case ND_LT:
    emit("  cmp rax, rdi");
    emit("  setl al");
    emit("  movzb rax, al");
    break;
  case ND_LE:
    emit("  cmp rax, rdi");
    emit("  setle al");
    emit("  movzb rax, al");
    break;
  }

  emit("  push rax");
}
