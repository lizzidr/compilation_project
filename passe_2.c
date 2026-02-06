
#include <stdio.h>

#include "defs.h"
#include "passe_2.h"
#include "miniccutils.h"


extern int trace_level;
static int in_print_context = 0;



void gen_code_passe_2(node_t root) {
    if (root == NULL) return;

    switch(root->nature){
        case(NODE_PROGRAM):
        {
            int32_t strings_number = get_global_strings_number();

            // Data section
            create_data_sec_inst(); // Création de la section .data
            gen_code_passe_2(root->opr[0]); // declarations
            for (int32_t i = 0; i < strings_number; i++) {
                create_asciiz_inst(NULL, get_global_string(i)); // chaines de caractères
            }

            // Text section
            create_text_sec_inst(); // Création de la section .text
            gen_code_passe_2(root->opr[1]); // main
            
            // Syscall exit
            create_ori_inst(2, 0, 10); 
            create_syscall_inst();
            break;
        }

        case(NODE_LIST):
        {
            printf_level(5, "PASSE 2 NODE_LIST \n");
            if (root->opr[0] != NULL)
                gen_code_passe_2(root->opr[0]);
            if (root->opr[1] != NULL)
                gen_code_passe_2(root->opr[1]);
            break;
        }

        case(NODE_FUNC):
        {
            create_label_str_inst(root->opr[1]->ident); // label de la fonction main
            create_stack_allocation_inst();
            gen_code_passe_2(root->opr[2]);

            create_stack_deallocation_inst(root->offset);
            break;
        }

        case(NODE_BLOCK):
        {
            printf_level(5, "PASSE 2 NODE_BLOCK \n");
            if (root->opr[0] != NULL)
                gen_code_passe_2(root->opr[0]);
            if (root->opr[1] != NULL)
                gen_code_passe_2(root->opr[1]);
            break;
        }

        case(NODE_DECLS):
        {
            printf_level(5, "PASSE 2 NODE_DECLS \n");
            if (root->opr[1] != NULL)
                gen_code_passe_2(root->opr[1]); // liste de NODE_DECL
            break;
        }

        case(NODE_DECL):
        {
            printf_level(5, "PASSE 2 NODE_DECL \n");
            int32_t registre;
            registre = get_current_reg();

            if(root->opr[0]->global_decl){ // declaration globale = ajouter dans .data
                if(root->opr[1] != NULL){
                    create_word_inst(root->opr[0]->ident, root->opr[1]->value);
                }
                else {
                    create_word_inst(root->opr[0]->ident, 0);
                }
            }   
            else {
                // Initialisation de la variable
                if(root->opr[1] != NULL) { // Si la variable est initialisée
                    // Initialisation par une constante ou par une expression
                    gen_code_passe_2(root->opr[1]); // Analyse de l'expression
                    create_sw_inst(registre, root->opr[0]->offset, 29);
                }
            }
            break; 
            }

        case NODE_AFFECT: {
            printf_level(5, "PASSE 2 NODE_AFFECT \n");
            int32_t registre = get_current_reg();
            int32_t stack_registre = get_stack_reg();
            int32_t tmp_reg;
            // Analyse expression d'affectation
            gen_code_passe_2(root->opr[1]); 
            if (root->opr[0]->decl_node->global_decl) {
                allocate_reg();
                tmp_reg = get_current_reg();
                create_lui_inst(tmp_reg, 0x1001);
                create_sw_inst(registre, root->opr[0]->decl_node->offset, tmp_reg);
                release_reg();
            } else {
                create_sw_inst(registre, root->opr[0]->offset, stack_registre);
            }

            
            break;
        }
        
        case (NODE_INTVAL):
        case (NODE_BOOLVAL): {
            printf_level(5, "PASSE 2 NODE_VAL \n");
            int32_t registre = get_current_reg();
            if ((root->value >= 0) && (root->value <= 0xFFFF)) {
                create_ori_inst(registre, 0, (int32_t)(root->value & 0xFFFF));
            } else {
                create_lui_inst(registre, (int32_t)((root->value >> 16) & 0xFFFF));
                create_ori_inst(registre, registre, (int32_t)(root->value & 0xFFFF));
            }
            break;
        }

        case (NODE_TYPE):{
            break; 
        }

        case (NODE_IDENT): {
            printf_level(5, "PASSE 2 NODE_IDENT \n");

            int32_t registre;
            if (in_print_context) registre = 4;
            else registre = get_current_reg();

            if (root->global_decl) {
                create_lui_inst(registre, 0x1001);
                create_lw_inst(registre, root->decl_node->offset, registre);
            } else {
                create_lw_inst(registre, root->offset, get_stack_reg());
            }
            
            // uniquement si on est dans print
            if (in_print_context) {
                // syscall print int
                create_ori_inst(2, 0, 1); 
                create_syscall_inst();
            }
            
            break;
        }

        case NODE_STRINGVAL:
        {
            printf_level(5, "PASSE 2 NODE_STRINGVAL \n");
            if (in_print_context = 0)
                break; // ce cas n'existe même pas anyway

            create_lui_inst(4, 0x1001);
            create_ori_inst(4, 4, root->offset);

            // syscall print string
            create_ori_inst(2, 0, 4); 
            create_syscall_inst();

            break;
        }

        /* -------- instructions de contrôle -------- */
        case NODE_IF: {
            int32_t Lelse = get_new_label();
            int32_t Lend  = get_new_label();

            // condition
            gen_code_passe_2(root->opr[0]);      // résultat dans $8
            create_beq_inst(8, 0, Lelse);        // si faux -> else

            // then
            gen_code_passe_2(root->opr[1]);

            // s'il y a un else, on saute la partie else après le then
            if (root->nops == 3) {
                create_j_inst(Lend);
            }

            // else
            create_label_inst(Lelse);
            if (root->nops == 3) {
                gen_code_passe_2(root->opr[2]);
                create_label_inst(Lend);
            }

            break;
        }

        case NODE_WHILE: {
            int32_t Lcond = get_new_label();
            int32_t Lend  = get_new_label();

            create_label_inst(Lcond);

            gen_code_passe_2(root->opr[0]);          // condition
            int32_t reg = get_current_reg();
            create_beq_inst(reg, 0, Lend);

            gen_code_passe_2(root->opr[1]);          // body

            create_j_inst(Lcond);
            create_label_inst(Lend);
            break;
        }

        case NODE_FOR: {
            printf_level(5, "PASSE 2 NODE_FOR \n");
            int32_t Lcond = get_new_label();
            int32_t Lend  = get_new_label();

            // init
            if (root->opr[0] != NULL) gen_code_passe_2(root->opr[0]);

            // label condition
            create_label_inst(Lcond);

            // condition
            if (root->opr[1] != NULL) {
                gen_code_passe_2(root->opr[1]);      // $8 = 0/1
                create_beq_inst(8, 0, Lend);         // si faux -> sortir
            }

            // block / incrementation
            
            if (root->opr[3] != NULL) gen_code_passe_2(root->opr[3]);
            
            if (root->opr[2] != NULL) gen_code_passe_2(root->opr[2]);

            // retour au test
            create_j_inst(Lcond);

            // sortie
            create_label_inst(Lend);
            break;
        }
        case(NODE_DOWHILE):{
            int32_t Lstart = get_new_label();

            // début de la boucle
            create_label_inst(Lstart);

            // body
            gen_code_passe_2(root->opr[0]);

            // condition
            gen_code_passe_2(root->opr[1]);   // résultat dans $8 (0 ou 1)

            // si condition vraie -> on reboucle
            create_bne_inst(8, 0, Lstart);

            break;
        }

        case (NODE_PRINT): {
            printf_level(5, "PASSE 2 NODE_PRINT \n");
            // Analyser l'enfant du noeud (LIST ou IDENT ou STRING)
            in_print_context = 1;
            gen_code_passe_2(root->opr[0]);
            in_print_context = 0;
            break;
        }

        /* -------- opérations arithmétiques -------- */
        case NODE_PLUS: {
            printf_level(5, "analyse passe 2 NODE_PLUS \n");
            gen_code_passe_2(root->opr[0]); 
            int32_t reg_left = get_current_reg();

            //cas ou pas de registre dispo
            if (!reg_available()) {
                // on passe par la pile
                push_temporary(reg_left); 
                release_reg(); 

                gen_code_passe_2(root->opr[1]); 
                int32_t reg_right = get_current_reg();
                
                allocate_reg();
                int32_t reg_restore = get_restore_reg(); 
                pop_temporary(reg_restore);

                create_addu_inst(reg_right, reg_restore, reg_right);

                release_reg(); //liberer reg_store
            }
            else {
                //cas normal
                allocate_reg(); 
                gen_code_passe_2(root->opr[1]); 
                int32_t reg_right = get_current_reg();

                create_addu_inst(reg_left, reg_left, reg_right); 
              
                release_reg();
            }
            break;
        }

        case NODE_MINUS: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                create_subu_inst(reg_right, reg_restore, reg_right);
            } else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                create_subu_inst(reg_left, reg_left, reg_right);
                release_reg();
            }
            break;
        }

        case NODE_MUL: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                create_mult_inst(reg_restore, reg_right);
                create_mflo_inst(reg_right);
            } else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                create_mult_inst(reg_left, reg_right);
                create_mflo_inst(reg_left);
                release_reg();
            }
            break;
        }

        case NODE_DIV: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                create_div_inst(reg_restore, reg_right);
                create_teq_inst(reg_right, 0);
                create_mflo_inst(reg_right);
            } else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                create_div_inst(reg_left, reg_right);
                create_teq_inst(reg_right, 0);
                create_mflo_inst(reg_left);
                release_reg();
            }
            break;
        }

        case NODE_MOD: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                create_div_inst(reg_restore, reg_right);
                create_teq_inst(reg_right, 0);
                create_mfhi_inst(reg_right);
            } else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                create_div_inst(reg_left, reg_right);
                create_teq_inst(reg_right, 0);
                create_mfhi_inst(reg_left);
                release_reg();
            }
            break;
        }

        /* -------- comparaisons -------- */
        case NODE_LT: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            // gestion Spill
            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]); // Droite
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                //reg_restore < reg_right
                create_slt_inst(reg_right, reg_restore, reg_right);
            } 
            else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]); // Droite
                int32_t reg_right = get_current_reg();

                //reg_left < reg_right
                create_slt_inst(reg_left, reg_left, reg_right);
                
                release_reg();
            }
            break;
        }

        case NODE_GT: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                // reg_right < reg_restore
                create_slt_inst(reg_right, reg_right, reg_restore); 
            } 
            else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                // reg_right < reg_left
                create_slt_inst(reg_left, reg_right, reg_left);
                
                release_reg();
            }
            break;
        }

        case NODE_LE: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                // reg_restore <= reg_right <=> !(reg_right < reg_restore)
                create_slt_inst(reg_right, reg_right, reg_restore);
                create_xori_inst(reg_right, reg_right, 1);
            } 
            else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                //reg_left <= reg_right <=> !(reg_right < reg_left)
                create_slt_inst(reg_left, reg_right, reg_left);
                create_xori_inst(reg_left, reg_left, 1);
                
                release_reg();
            }
            break;
        }

        case NODE_GE: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                //reg_restore >= reg_right <=> !(reg_restore < reg_right)
                create_slt_inst(reg_right, reg_restore, reg_right);
                create_xori_inst(reg_right, reg_right, 1);
            } 
            else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                //reg_left >= reg_right <=> !(reg_left < reg_right)
                create_slt_inst(reg_left, reg_left, reg_right);
                create_xori_inst(reg_left, reg_left, 1);
                
                release_reg();
            }
            break;
        }


        case NODE_EQ: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                create_xor_inst(reg_right, reg_restore, reg_right);
                create_sltiu_inst(reg_right, reg_right, 1);
            } 
            else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                create_xor_inst(reg_left, reg_left, reg_right);
                create_sltiu_inst(reg_left, reg_left, 1);
                
                release_reg();
            }
            break;
        }

        case NODE_NE: {
            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            if (!reg_available()) {
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                create_xor_inst(reg_right, reg_restore, reg_right);
                create_sltiu_inst(reg_right, reg_right, 1);
                create_xori_inst(reg_right, reg_right, 1);
            } 
            else {
                allocate_reg();
                gen_code_passe_2(root->opr[1]);
                int32_t reg_right = get_current_reg();

                create_xor_inst(reg_left, reg_left, reg_right);
                create_sltiu_inst(reg_left, reg_left, 1);
                create_xori_inst(reg_left, reg_left, 1);
                
                release_reg();
            }
            break;
        }



        /* -------- Opérations Binaires Logiques & Bitwise -------- */
        case NODE_AND:  // &&
        case NODE_OR:   // ||
        case NODE_BAND: // &
        case NODE_BOR:  // |
        case NODE_BXOR: // ^
        case NODE_SLL:  // <<
        case NODE_SRA:  // >> 
        case NODE_SRL:  // >>> 
        {

            gen_code_passe_2(root->opr[0]);
            int32_t reg_left = get_current_reg();

            // spill
            if (!reg_available()) {
                // --- CAS SPILL ---
                push_temporary(reg_left);
                release_reg();

                gen_code_passe_2(root->opr[1]); //DROITE
                int32_t reg_right = get_current_reg();
                
                int32_t reg_restore = get_restore_reg();
                pop_temporary(reg_restore);

                switch(root->nature) {
                    case NODE_AND:  create_and_inst(reg_right, reg_restore, reg_right); break;
                    case NODE_OR:   create_or_inst(reg_right, reg_restore, reg_right); break;
                    
                    case NODE_BAND: create_and_inst(reg_right, reg_restore, reg_right); break;
                    case NODE_BOR:  create_or_inst(reg_right, reg_restore, reg_right); break;
                    case NODE_BXOR: create_xor_inst(reg_right, reg_restore, reg_right); break;

                    
                    case NODE_SLL:  create_sllv_inst(reg_right, reg_restore, reg_right); break;
                    case NODE_SRA:  create_srav_inst(reg_right, reg_restore, reg_right); break;
                    case NODE_SRL:  create_srlv_inst(reg_right, reg_restore, reg_right); break;
                    default: break;
                }
            } 
            else {
                // cas normal 
                allocate_reg();
                gen_code_passe_2(root->opr[1]); // Générer DROITE
                int32_t reg_right = get_current_reg();

                
                switch(root->nature) {
                    case NODE_AND:  create_and_inst(reg_left, reg_left, reg_right); break;
                    case NODE_OR:   create_or_inst(reg_left, reg_left, reg_right); break;
                    
                    case NODE_BAND: create_and_inst(reg_left, reg_left, reg_right); break;
                    case NODE_BOR:  create_or_inst(reg_left, reg_left, reg_right); break;
                    case NODE_BXOR: create_xor_inst(reg_left, reg_left, reg_right); break;

                    case NODE_SLL:  create_sllv_inst(reg_left, reg_left, reg_right); break;
                    case NODE_SRA:  create_srav_inst(reg_left, reg_left, reg_right); break;
                    case NODE_SRL:  create_srlv_inst(reg_left, reg_left, reg_right); break;
                    default: break;
                }
                
                release_reg(); 
            }
            break;
        }


        /* -------- Opérations Unaires -------- */
        case NODE_NOT: { 
            gen_code_passe_2(root->opr[0]);
            int32_t reg = get_current_reg();
            // 1 devient 0, 0 devient 1
            create_xori_inst(reg, reg, 1);
            break;
        }

        case NODE_BNOT: { 
            gen_code_passe_2(root->opr[0]);
            int32_t reg = get_current_reg();
            create_nor_inst(reg, reg, 0);
            break;
        }
    }
}
  