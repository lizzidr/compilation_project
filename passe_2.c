
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
                
                    gen_code_passe_2(root->opr[1]); // Analyse du 
                    create_sw_inst(registre, root->opr[0]->offset, 29);
                    }
                }
            break; 
            }

        case(NODE_AFFECT):{
            gen_code_passe_2(root->opr[1]);

            if (root->opr[0]->decl_node->global_decl){
                create_lui_inst(9, 0x1001);
                create_sw_inst(8, root->opr[0]->offset,9);
            }
            else {
                create_sw_inst(8, root->opr[0]->offset, 29); //variable locale = adresse 29
            }
            break;

            }
            
            
        
        case(NODE_IDENT):
        { 
            if (root->decl_node->global_decl){
                create_lui_inst(8, 0x1001);
                create_lw_inst(8, root->offset,8);
            }
            else{
                create_lw_inst(8,root->offset,29);
            }
            break;
        }

        case(NODE_TYPE):{
            break;
        }

        case (NODE_INTVAL):{
            create_ori_inst(8,0,root->value);
            break;
        }

        case(NODE_BOOLVAL):{
            create_ori_inst(8,0,root->value);
            break;
        }

        
        case(NODE_STRINGVAL):
        {
            create_lui_inst(8,0x1001);
            create_addiu_inst(8, 8, root->offset);  
            break;
        }



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
    }
}
  

