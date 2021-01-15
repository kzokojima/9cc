#include "codegen.h"

#include <string.h>

#include "lib.h"

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

void gen_load(Type *type) {
  if (get_type_size(type->ty) == 8)
    emit("  mov rax, [rax]");
  else if (type->ty == kTypeInt)
    emit("  movsx rax, DWORD PTR [rax]");
  else if (type->ty == kTypeUInt)
    emit("  mov eax, [rax]");
  else if (type->ty == kTypeLLong)
    emit("  mov rax, [rax]");
  else if (type->ty == kTypeULLong)
    emit("  mov rax, [rax]");
  else if (type->ty == kTypeShort)
    emit("  movsx rax, WORD PTR [rax]");
  else if (type->ty == kTypeUShort)
    emit("  movzx rax, WORD PTR [rax]");
  else
    emit("  movsx eax, BYTE PTR [rax]");
}

void gen(Node *node) {
  static int s_lavel_no = 1;
  static int s_in_function = 0;
  static int s_lavel_stack[10];
  static int s_lavel_stack_index = -1;
  int lavel_no;
  Node *current;
  char *registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  char *registers_32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
  int registers_len = sizeof registers / sizeof registers[0];
  int i;
  Type *type;

  switch (node->kind) {
    case kNodeNum:
      emit("  mov rax, %llu", node->val);
      emit("  push rax");
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
        gen_load(node->type);
      }
      emit("  push rax");
      return;
    case kNodeAssign:
      if (node->lhs->kind == kNodeDeref) {
        if (node->lhs->lhs->kind == kNodeLocalVar ||
            node->lhs->lhs->kind == kNodeGlobalVar) {
          if (node->lhs->lhs->type->ty == kTypeArray) {
            // *array = ...;
            gen_val(node->lhs->lhs);
            type = node->lhs->lhs->type->ptr_to;
          } else {
            // *pointer = ...;
            gen(node->lhs->lhs);
            type = node->lhs->lhs->type;
          }
        } else if (node->lhs->lhs->kind == kNodeAdd) {
          // *(array + 1) = ...;
          gen_val(node->lhs->lhs->lhs);
          gen(node->lhs->lhs->rhs);
          emit("  pop rdi");
          emit("  pop rax");
          emit("  imul rdi, %d",
               get_type_size(node->lhs->lhs->lhs->type->ptr_to->ty));
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
      else if (get_type_size(type->ty) == 2)
        emit("  mov [rax], di");
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
    case kNodeNOP:
      emit("  push rax");
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
      s_lavel_stack[++s_lavel_stack_index] = lavel_no;
      gen(node->rhs);
      s_lavel_stack_index--;
      emit("  jmp Lbegin%d", lavel_no);
      emit("Lend%d:", lavel_no);
      return;
    case kNodeDoWhile:
      lavel_no = s_lavel_no;
      s_lavel_no++;
      emit("  jmp Lbody%d", lavel_no);
      emit("Lbegin%d:", lavel_no);
      gen(node->lhs);
      emit("  pop rax");
      emit("  cmp rax, 0");
      emit("  je  Lend%d", lavel_no);
      s_lavel_stack[++s_lavel_stack_index] = lavel_no;
      emit("Lbody%d:", lavel_no);
      gen(node->rhs);
      s_lavel_stack_index--;
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
      s_lavel_stack[++s_lavel_stack_index] = lavel_no;
      gen(node->rhs);
      s_lavel_stack_index--;
      gen(node->for_expression3);
      emit("  jmp Lbegin%d", lavel_no);
      emit("Lend%d:", lavel_no);
      return;
    case kNodeSwitch:
      lavel_no = s_lavel_no;
      s_lavel_no++;

      gen(node->lhs);

      // jump
      emit("  pop rax");
      Node *case_node = node->rhs;
      while (case_node) {
        if (case_node->kind == kNodeSwitchCase) {
          emit("  cmp rax, %d", case_node->val);
          emit("  je  Lswitch%d_case%d", lavel_no, case_node->val);
        } else {
          emit("  jmp  Lswitch%d_default", lavel_no);
        }
        case_node = case_node->lhs;
      }
      emit("  jmp  Lend%d", lavel_no);

      // cases
      case_node = node->rhs;
      s_lavel_stack[++s_lavel_stack_index] = lavel_no;
      while (case_node) {
        if (case_node->kind == kNodeSwitchCase) {
          emit("Lswitch%d_case%d:", lavel_no, case_node->val);
        } else {
          emit("Lswitch%d_default:", lavel_no);
        }
        current = case_node->rhs;
        while (current) {
          gen(current);
          emit("  pop rax");
          current = current->next;
        }
        case_node = case_node->lhs;
      }
      s_lavel_stack_index--;
      emit("Lend%d:", lavel_no);
      emit("  push rax");

      return;
    case kNodeBreak:
      emit("  jmp Lend%d", s_lavel_stack[s_lavel_stack_index]);
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
      for (; 0 < i; i--) {
        emit("  pop %s", registers[i - 1]);
        current = current->next;
      }
      char *plt = "@PLT";
      for (i = 0; code[i]; i++) {
        if (code[i]->kind == kNodeFunc && !strncmp(code[i]->name, node->name, node->len)) {
          plt = "";
        }
      }
      emit("  push r15");
      emit("  mov r15, rsp");
      emit("  and spl, 0xF0");
      emit("  mov al, 0");
      emit("  call %1$.*2$s%3$s", node->name, node->len, plt);
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
    case kNodeDeref: {
      Node *var_node;
      gen(node->lhs);
      emit("  pop rax");
      if (node->lhs->kind == kNodeAdd) {
        var_node = node->lhs->lhs;
      } else {
        var_node = node->lhs;
      }
      if (var_node->kind == kNodeLocalVar || var_node->kind == kNodeGlobalVar) {
        gen_load(var_node->type->ptr_to);
      } else {
        emit("  mov rax, [rax]");
      }
      emit("  push rax");
      return;
    }
    case kNodeLogicalNot:
      gen(node->lhs);
      emit("  xor rax, rax");
      emit("  pop rdi");
      emit("  test rdi, rdi");
      emit("  sete al");
      emit("  push rax");
      return;
    case kNodeVarDef:
      if (s_in_function) {
        if (node->lhs) {
          // 初期化
          if (node->lhs->lhs->type->ty == kTypeArray &&
              node->lhs->rhs->kind == kNodeInitializerList) {
            Node *cur = node->lhs->rhs->next;
            int offset = node->lhs->lhs->offset;
            int size = get_type_size(node->lhs->lhs->type->ptr_to->ty);
            while (cur != NULL) {
              switch (size) {
                case 1:
                  emit("  mov BYTE PTR [rbp-%d], %d", offset, cur->val);
                  break;
                case 2:
                  emit("  mov WORD PTR [rbp-%d], %d", offset, cur->val);
                  break;
                case 4:
                  emit("  mov DWORD PTR [rbp-%d], %d", offset, cur->val);
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
        int is_bss = 0;
        int bss_size;
        if (node->type->ty == kTypeArray) {
          if (node->lhs) {
            // 初期化子リスト
            emit(".text");
            emit(".globl %1$.*2$s", node->name, node->len);
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
          } else if (!node->is_extern) {
            is_bss = 1;
            bss_size = get_type_size(node->type->ptr_to->ty) * node->type->array_size;
          }
        } else if (node->type->ty == kTypePtr) {
          if (node->lhs) {
            // 初期化
            emit(".text");
            emit(".globl %1$.*2$s", node->name, node->len);
            if (node->lhs->rhs->kind == kNodeString) {
              emit("%1$.*2$s:\n  .quad .LC%3$d", node->name, node->len,
                   node->lhs->rhs->string_constant->index);
            } else if (node->lhs->rhs->kind == kNodeAddr) {
              if (node->lhs->rhs->lhs->kind == kNodeStructMember) {
                emit("%1$.*2$s:\n  .quad %3$.*4$s+%5$d", node->name, node->len,
                     node->lhs->rhs->lhs->lhs->name,
                     node->lhs->rhs->lhs->lhs->len,
                     node->lhs->rhs->lhs->offset);
              } else {
                emit("%1$.*2$s:\n  .quad %3$.*4$s", node->name, node->len,
                     node->lhs->rhs->lhs->name, node->lhs->rhs->lhs->len);
              }
            }
          } else if (!node->is_extern) {
            is_bss = 1;
            bss_size = get_type_size(node->type->ty);
          }
        } else {
          if (node->lhs) {
            // 初期化
            int size = get_type_size(node->lhs->lhs->type->ty);
            emit(".data");
            emit(".globl %1$.*2$s", node->name, node->len);
            switch (size) {
              case 1:
                emit("%1$.*2$s:\n  .byte %3$d", node->name, node->len,
                     node->lhs->rhs->val);
                break;
              case 4:
                emit("%1$.*2$s:\n  .long %3$d", node->name, node->len,
                     node->lhs->rhs->val);
                break;
              case 8:
                emit("%1$.*2$s:\n  .quad %3$d", node->name, node->len,
                     node->lhs->rhs->val);
                break;
            }
          } else if (!node->is_extern) {
            is_bss = 1;
            bss_size = get_type_size_by_type(node->type);
          }
        }
        if (is_bss) {
          emit(".text");
          emit(".comm %1$.*2$s,%3$d,%3$d", node->name, node->len, bss_size);
        }
      }
      return;
    case kNodeString:
      emit("  lea rax, .LC%d[rip]", node->string_constant->index);
      emit("  push rax");
      return;
    case kNodeStruct:
      return;
    case kNodeTernaryConditional:
      lavel_no = s_lavel_no;
      s_lavel_no++;

      gen(node->lhs);
      emit("  pop rax");
      emit("  cmp rax, 0");
      emit("  je  Lfalse%d", lavel_no);
      emit("  jmp  Ltrue%d", lavel_no);

      emit("Ltrue%d:", lavel_no);
      gen(node->rhs);
      emit("  jmp  Lelse%d", lavel_no);

      emit("Lfalse%d:", lavel_no);
      gen(node->rhs->next);

      emit("Lelse%d:", lavel_no);

      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  emit("  pop rdi");
  emit("  pop rax");

  switch (node->kind) {
    case kNodeAdd:
      if ((node->lhs->kind == kNodeLocalVar ||
           node->lhs->kind == kNodeGlobalVar) &&
          (node->lhs->type->ty == kTypeArray ||
           node->lhs->type->ty == kTypePtr)) {
        emit("  imul rdi, %d", get_type_size(node->lhs->type->ptr_to->ty));
      } else if ((node->rhs->kind == kNodeLocalVar ||
                  node->rhs->kind == kNodeGlobalVar) &&
                 (node->rhs->type->ty == kTypeArray ||
                  node->rhs->type->ty == kTypePtr)) {
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
    case kNodeLogicalOr:
      emit("  or rax, rdi");
      emit("  cmp rax, 0");
      emit("  setne al");
      emit("  movzb rax, al");
      break;
    case kNodeLogicalAnd:
      emit("  and rax, rdi");
      emit("  cmp rax, 0");
      emit("  setne al");
      emit("  movzb rax, al");
      break;
  }

  emit("  push rax");
}
