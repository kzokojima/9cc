#include "9cc.h"

void emit(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(output, fmt, ap);
  fprintf(output, "\n");
}

void gen_string_constants() {
  StringConstant *str = string_constants;
  emit(".text");
  emit(".section	.rodata");
  while (str) {
    emit(".LC%d:\n  .string \"%2$.*3$s\"", str->index, str->str, str->len);
    str = str->next;
  }
}

void gen_lval(Node *node) {
  if (node->kind != kNodeLocalVar) {
    error("変数ではありません");
  }

  emit("# var:%1$.*2$s", node->name, node->len);
  emit("  mov rax, rbp");
  emit("  sub rax, %d", node->offset);
  emit("  push rax");
}

void gen_gval(Node *node) {
  if (node->kind != kNodeGlobalVar) {
    error("変数ではありません");
  }

  emit("  lea rax, %1$.*2$s[rip]", node->name, node->len);
  emit("  push rax");
}

void gen_val(Node *node) {
  switch (node->kind) {
  case kNodeLocalVar:
    gen_lval(node);
    break;
  case kNodeGlobalVar:
    gen_gval(node);
    break;
  case kNodeStructMember:
  case kNodeStructPointerMember:
    gen_val(node->lhs);
    emit("  pop rax");
    if (node->kind == kNodeStructPointerMember) {
      emit("  mov rax, [rax]");
    }
    emit("  add rax, %d", node->offset);
    emit("  push rax");
    break;
  default:
    error("変数ではありません");
  }
}

void gen(Node *node) {
  static int s_lavel_no = 1;
  static int s_in_function = 0;
  int lavel_no;
  Node *current;
  char *registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  char *registers_32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
  int registers_len = sizeof registers / sizeof registers[0];
  int i;
  Type *type;

  switch (node->kind) {
  case kNodeNum:
    emit("  push %d", node->val);
    return;
  case kNodeLocalVar:
  case kNodeGlobalVar:
  case kNodeStructMember:
  case kNodeStructPointerMember:
    gen_val(node);
    emit("  pop rax");
    if (node->type->ty == kTypeArray) {
        // do nothing
    } else {
      if (get_type_size(node->type->ty) == 8)
        emit("  mov rax, [rax]");
      else if (node->type->ty == kTypeInt)
        emit("  movsx rax, WORD PTR [rax]");
      else if (node->type->ty == kTypeUInt)
        emit("  mov eax, [rax]");
      else
        emit("  movsx eax, BYTE PTR [rax]");
    }
    emit("  push rax");
    return;
  case kNodeAssign:
    if (node->lhs->kind == kNodeDeref) {
      if (node->lhs->lhs->kind == kNodeLocalVar || node->lhs->lhs->kind == kNodeGlobalVar) {
        if (node->lhs->lhs->type->ty == kTypeArray) {
          // *array = ...;
          gen_val(node->lhs->lhs);
        } else {
          // *pointer = ...;
          gen(node->lhs->lhs);
        }
        type = node->lhs->lhs->type;
      } else if (node->lhs->lhs->kind == kNodeAdd) {
        // *(array + 1) = ...;
        gen_val(node->lhs->lhs->lhs);
        gen(node->lhs->lhs->rhs);
        emit("  pop rdi");
        emit("  pop rax");
        emit("  imul rdi, %d", get_type_size(node->lhs->lhs->lhs->type->ptr_to->ty));
        emit("  add rax, rdi");
        emit("  push rax");
        type = node->lhs->lhs->lhs->type->ptr_to;
      } else {
        error("代入式が不正です");
      }
    } else {
      gen_val(node->lhs);
      type = node->lhs->type;
    }
    gen(node->rhs);
    emit("  pop rdi");
    emit("  pop rax");
    if (get_type_size(type->ty) == 8)
      emit("  mov [rax], rdi");
    else if (get_type_size(type->ty) == 4)
      emit("  mov [rax], edi");
    else
      emit("  mov [rax], dil");
    emit("  push rdi");
    return;
  case kNodeReturn:
    gen(node->lhs);
    emit("  pop rax");
    emit("  mov rsp, rbp");
    emit("  pop rbp");
    emit("  ret");
    return;
  case kNodeIf:
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
  case kNodeWhile:
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
  case kNodeFor:
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
  case kNodeBlock:
    current = node->lhs;
    while (current) {
      gen(current);
      emit("  pop rax");
      current = current->next;
    }
    emit("  push rax");
    return;
  case kNodeFuncCall: {
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
    emit("  mov al, 0");
    emit("  call %1$.*2$s", node->name, node->len);
    emit("  mov rsp, r15");
    emit("  pop r15");
    emit("  push rax");
    return;
  }
  case kNodeFunc:
    s_in_function = 1;
    emit(".text");
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
    emit("  mov rsp, rbp");
    emit("  pop rbp");
    emit("  mov rax, 0");
    emit("  ret");
    s_in_function = 0;
    return;
  case kNodeAddr:
    gen_val(node->lhs);
    return;
  case kNodeDeref:
    gen(node->lhs);
    emit("  pop rax");
    emit("  mov rax, [rax]");
    emit("  push rax");
    return;
  case kNodeVarDef:
    if (s_in_function) {
      if (node->lhs) {
        // 初期化
        if (node->lhs->lhs->type->ty == kTypeArray &&  node->lhs->rhs->kind == kNodeInitializerList) {
          Node *cur = node->lhs->rhs->next;
          int offset = node->lhs->lhs->offset;
          int size = get_type_size(node->lhs->lhs->type->ptr_to->ty);
          while (cur != NULL) {
            switch (size) {
            case 1:
              emit("  mov BYTE PTR [rbp-%d], %d", offset, cur->val);
              break;
            case 4:
              emit("  mov WORD PTR [rbp-%d], %d", offset, cur->val);
              break;
            case 8:
              emit("  mov QWORD PTR [rbp-%d], %d", offset, cur->val);
              break;
            default:
              error("不正な初期化です");
            }
            offset -= size;
            cur = cur->next;
          }
          emit("  push rax");
        } else {
          gen(node->lhs);
        }
      } else {
        emit("  push rax");
      }
    } else {
      // グローバル変数
      if (node->type->ty == kTypeArray) {
        if (node->lhs) {
          // 初期化子リスト
          emit(".text");
          emit("%1$.*2$s:", node->name, node->len);
          Node *node_num = node->lhs->rhs->next;
          int size = get_type_size(node->type->ptr_to->ty);
          while (node_num) {
            switch (size) {
            case 1:
              emit("  .byte %d", node_num->val);
              break;
            case 4:
              emit("  .long %d", node_num->val);
              break;
            case 8:
              emit("  .quad %d", node_num->val);
              break;
            }
            node_num = node_num->next;
          }
        } else {
          emit(".bss");
          emit("%1$.*2$s:\n  .zero %3$d", node->name, node->len, get_type_size(node->type->ptr_to->ty) * node->type->array_size);
        }
      } else if (node->type->ty == kTypePtr) {
        if (node->lhs) {
          // 初期化
          emit(".text");
          if (node->lhs->rhs->kind == kNodeString) {
            emit("%1$.*2$s:\n  .quad .LC%3$d", node->name, node->len, node->lhs->rhs->string_constant->index);
          } else if (node->lhs->rhs->kind == kNodeAddr) {
            if (node->lhs->rhs->lhs->kind == kNodeStructMember) {
              emit("%1$.*2$s:\n  .quad %3$.*4$s+%5$d", node->name, node->len, node->lhs->rhs->lhs->lhs->name, node->lhs->rhs->lhs->lhs->len,
                node->lhs->rhs->lhs->offset);
            } else {
              emit("%1$.*2$s:\n  .quad %3$.*4$s", node->name, node->len, node->lhs->rhs->lhs->name, node->lhs->rhs->lhs->len);
            }
          }
        } else {
          emit(".bss");
          emit("%1$.*2$s:\n  .zero %3$d", node->name, node->len, get_type_size(node->type->ty));
        }
      } else  {
        if (node->lhs) {
          // 初期化
          int size = get_type_size(node->lhs->lhs->type->ty);
          emit(".text");
          switch (size) {
          case 1:
            emit("%1$.*2$s:\n  .byte %3$d", node->name, node->len, node->lhs->rhs->val);
            break;
          case 4:
            emit("%1$.*2$s:\n  .long %3$d", node->name, node->len, node->lhs->rhs->val);
            break;
          case 8:
            emit("%1$.*2$s:\n  .quad %3$d", node->name, node->len, node->lhs->rhs->val);
            break;
          }
        } else {
          emit(".bss");
          emit("%1$.*2$s:\n  .zero %3$d", node->name, node->len, get_type_size_by_type(node->type));
        }
      }
    }
    return;
  case kNodeString:
    emit("  lea rax, .LC%d[rip]", node->string_constant->index);
    emit("  push rax");
    return;
  case kNodeStruct:
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  emit("  pop rdi");
  emit("  pop rax");

  switch (node->kind) {
  case kNodeAdd:
    if ((node->lhs->kind == kNodeLocalVar || node->lhs->kind == kNodeGlobalVar) && (node->lhs->type->ty == kTypeArray || node->lhs->type->ty == kTypePtr)) {
      emit("  imul rdi, %d", get_type_size(node->lhs->type->ptr_to->ty));
    } else if ((node->rhs->kind == kNodeLocalVar || node->rhs->kind == kNodeGlobalVar) && (node->rhs->type->ty == kTypeArray || node->rhs->type->ty == kTypePtr)) {
      emit("  imul rax, %d", get_type_size(node->rhs->type->ptr_to->ty));
    }
    emit("  add rax, rdi");
    break;
  case kNodeSub:
    emit("  sub rax, rdi");
    break;
  case kNodeMul:
    emit("  imul rax, rdi");
    break;
  case kNodeDiv:
    emit("  cqo");
    emit("  idiv rdi");
    break;
  case kNodeEq:
    emit("  cmp rax, rdi");
    emit("  sete al");
    emit("  movzb rax, al");
    break;
  case kNodeNe:
    emit("  cmp rax, rdi");
    emit("  setne al");
    emit("  movzb rax, al");
    break;
  case kNodeLt:
    emit("  cmp rax, rdi");
    emit("  setl al");
    emit("  movzb rax, al");
    break;
  case kNodeLe:
    emit("  cmp rax, rdi");
    emit("  setle al");
    emit("  movzb rax, al");
    break;
  }

  emit("  push rax");
}
