
#include <stdio.h>

#include "defs.h"
#include "passe_2.h"
#include "miniccutils.h"


extern int trace_level;

void gen_code_passe_2(node_t root) {
    if (root == NULL) return;

    switch(root->nature){
        case(NODE_PROGRAM):
        {
            create_data_sec_inst(); // Création de la section .data
            gen_code_passe_2(root->opr[0]); // declarations

            create_text_sec_inst(); // Création de la section .text
            gen_code_passe_2(root->opr[1]); // main
            break;
        }

        case(NODE_LIST):
        {
            if (root->opr[0])
                gen_code_passe_2(root->opr[0]);
            if (root->opr[1])
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
            gen_code_passe_2(root->opr[0]);
            gen_code_passe_2(root->opr[1]);
            break;
        }

        case(NODE_DECLS):
        {
            gen_code_passe_2(root->opr[1]); // liste de NODE_DECL
            break;
        }

        case(NODE_DECL):
        {
            int32_t registre;
            registre = get_num_registers();

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
            gen_code_passe_2(root->opr[1]);
            int32_t registre = get_current_reg();

 

            if (root->opr[0]->decl_node->global_decl) {
                allocate_reg();
                int32_t tmp_reg = get_current_reg();
                create_lui_inst(tmp_reg, 0x1001);
                create_sw_inst(registre, root->opr[0]->decl_node->offset, tmp_reg);
                release_reg();
            } else {
                create_sw_inst(registre, root->opr[0]->offset, get_stack_reg());
            }
            break;
        }
        
        case (NODE_INTVAL):
        case (NODE_BOOLVAL): {
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
            int32_t registre = get_current_reg();

            if (root->global_decl) {
                create_lui_inst(registre, 0x1001);
                create_lw_inst(registre, root->decl_node->offset, registre);
            } else {
                create_lw_inst(registre, root->offset, get_stack_reg());
            }
            break;
        }

        
        case(NODE_STRINGVAL):
        {
            
            break;
        }

        /* -------- instructions de contrôle -------- */
        case NODE_IF: {}
        case NODE_WHILE: {}
        case NODE_FOR: {
            int32_t Lcond = get_new_label();
            int32_t Lend  = get_new_label();

            // init
            if (root->opr[0]) gen_code_passe_2(root->opr[0]);

            // label condition
            create_label_inst(Lcond);

            // condition
            if (root->opr[1]) {
                gen_code_passe_2(root->opr[1]);      // $8 = 0/1
                create_beq_inst(8, 0, Lend);         // si faux -> sortir
            }

            // corps
            if (root->opr[3]) gen_code_passe_2(root->opr[3]);

            // incrément
            if (root->opr[2]) gen_code_passe_2(root->opr[2]);

            // retour au test
            create_j_inst(Lcond);

            // sortie
            create_label_inst(Lend);
            break;
        }


        case(NODE_DOWHILE):{}




        case(NODE_PRINT):{
            for (int i = 0; i < root->nops; i++) {
                gen_code_passe_2(root->opr[i]);
                create_addu_inst(4, 8, 0);
                if (root->opr[i]->nature == NODE_STRINGVAL) {
                create_ori_inst(2, 0, 4);   // syscall print_string
            } else {
                create_ori_inst(2, 0, 1);   // syscall print_int
            }
                create_syscall_inst();
                }
            break;
        
        }



        /* Opérateurs binaires (arithmétique, logique, etc.) */
        /* -------- opérations arithmétiques -------- */
        case NODE_PLUS: {
                gen_code_passe_2(root->opr[0]); // $8 = gauche

                if (root->opr[1] && root->opr[1]->nature == NODE_INTVAL) {
                    create_addiu_inst(9, 0, root->opr[1]->value);
                    create_addu_inst(8, 8, 9);
                } else {
                    create_addu_inst(9, 8, 0);      // $9 = gauche
                    gen_code_passe_2(root->opr[1]); // $8 = droite
                    create_addu_inst(8, 9, 8);      // $8 = gauche + droite
                }
                break;
        }

        case NODE_MINUS:{
         gen_code_passe_2(root->opr[0]); // $8 = gauche
                if (root->opr[1] && root->opr[1]->nature == NODE_INTVAL) {
                    create_addiu_inst(9, 0, root->opr[1]->value);
                    create_subu_inst(8, 8, 9);
                } else {
                    create_addu_inst(9, 8, 0);      // $9 = gauche
                    gen_code_passe_2(root->opr[1]); // $8 = droite
                    create_subu_inst(8, 9, 8);      // $8 = gauche - droite
                }
                break;   
        }

        case NODE_MUL:{
         gen_code_passe_2(root->opr[0]); // $8 = gauche

                if (root->opr[1] && root->opr[1]->nature == NODE_INTVAL) {
                    create_addu_inst(9, 8, 0); // $9 = gauche
                    create_addiu_inst(8, 0, root->opr[1]->value); //const
                    create_mult_inst(9, 8); //LO = $9*$8
                    create_mflo_inst(8); //$8 = LO
                } else {
                    create_addu_inst(9, 8, 0);      // $9 = gauche
                    gen_code_passe_2(root->opr[1]); // $8 = droite
                    create_mult_inst(8, 9);      // $8 = gauche * droite
                    create_mflo_inst(8);
                }
                break;   
        }

        case NODE_DIV:{
         gen_code_passe_2(root->opr[0]); // $8 = gauche

                if (root->opr[1] && root->opr[1]->nature == NODE_INTVAL) {
                    create_addu_inst(9, 8, 0); // $9 = gauche
                    create_addiu_inst(8, 0, root->opr[1]->value); //const
                    create_div_inst(9, 8); //LO = $9/$8
                    create_mflo_inst(8); //$8 = LO
                } else {
                    create_addu_inst(9, 8, 0);      // $9 = gauche
                    gen_code_passe_2(root->opr[1]); // $8 = droite
                    create_div_inst(8, 9);      // $8 = gauche / droite
                    create_mflo_inst(8);
                }
                break;    
        }

        case NODE_MOD: {
        gen_code_passe_2(root->opr[0]); // $8 = gauche

                if (root->opr[1] && root->opr[1]->nature == NODE_INTVAL) {
                    create_addu_inst(9, 8, 0); // $9 = gauche
                    create_addiu_inst(8, 0, root->opr[1]->value); //const
                    create_div_inst(9, 8); //HI = $9/$8
                    create_mfhi_inst(8); //$8 = HI
                } else {
                    create_addu_inst(9, 8, 0);      // $9 = gauche
                    gen_code_passe_2(root->opr[1]); // $8 = droite
                    create_div_inst(8, 9);      // $8 = gauche / droite
                    create_mfhi_inst(8);
                }
                break;    
        }

        /* -------- comparaisons -------- */
        case NODE_LT: {
            gen_code_passe_2(root->opr[0]);      
            create_addu_inst(9, 8, 0);           
            gen_code_passe_2(root->opr[1]);      
            create_slt_inst(8, 9, 8);            
            break;
        }

        case NODE_GT: {
            gen_code_passe_2(root->opr[0]);      
            create_addu_inst(9, 8, 0);           
            gen_code_passe_2(root->opr[1]);      
            create_slt_inst(8, 8, 9);        
            break;
        }

        case NODE_LE:{
            gen_code_passe_2(root->opr[0]);      
            create_addu_inst(9, 8, 0);          
            gen_code_passe_2(root->opr[1]);     
            create_slt_inst(8, 8, 9);            
            create_xori_inst(8, 8, 1);           // $8 = a <= b
            break; 
        }
        case NODE_GE: {
            gen_code_passe_2(root->opr[0]);      
            create_addu_inst(9, 8, 0);           
            gen_code_passe_2(root->opr[1]);      
            create_slt_inst(8, 9, 8);           
            create_xori_inst(8, 8, 1);           // $8 = a >= b
            break;
        }


        case NODE_EQ:{
            gen_code_passe_2(root->opr[0]);      
            create_addu_inst(9, 8, 0);           
            gen_code_passe_2(root->opr[1]);      
            create_xor_inst(8, 9, 8);            
            create_sltiu_inst(8, 8, 1);          // $8 = ($8 == 0) ? 1 : 0
            break;
        }

        case NODE_NE: {
            gen_code_passe_2(root->opr[0]);
            create_addu_inst(9, 8, 0);
            gen_code_passe_2(root->opr[1]);
            create_xor_inst(8, 9, 8);        // 0 si égaux
            create_sltiu_inst(8, 8, 1);      // 1 si égaux
            create_xori_inst(8, 8, 1);       // inverse => 1 si différents
            break;
        }
    }
}
  