
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
            }
            
            break;
        


        case(NODE_IDENT):
        {
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
        }


        case(NODE_STRINGVAL):
        {
            break;
        }
}
}
  

