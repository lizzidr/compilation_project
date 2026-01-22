
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
            create_text_sec_inst();
            gen_code_passe_2(root->opr[0]); // declarations
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
            if(root->opr[1])
            break;
        }

        case(NODE_IDENT):
        {
            
        }

        case(NODE_FUNC):
        {
            create_label_str_inst(root->opr[1]->ident); // label de la fonction main
            
            
            gen_code_passe_2(root->opr[2]);
            break;
        }

        case(NODE_BLOCK):
        {

        }
}
}
  

