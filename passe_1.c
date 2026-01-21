
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "defs.h"
#include "passe_1.h"
#include "miniccutils.h"


extern int yylineno;
extern int trace_level;


static void print_error(int line, const char* msg){
  printf("Error line %d : %s\n", yylineno, msg);
  exit(1);
}


void analyse_passe_1(node_t root, node_type type) {
    if (root == NULL) return;

    switch(root->nature){
        case(NODE_PROGRAM): {
            printf("Analyse passe 1 NODE_PROGRAM\n");
            push_global_context(); // A appeller uniquement si il y a NODE_DECLS?
            if (root->opr[0]!= NULL)
              analyse_passe_1(root->opr[0], type); // déclarations globales
            
            analyse_passe_1(root->opr[1], type); // main
            
            pop_context();
            break;
            }
        
        case(NODE_LIST):{
          printf("Analyse passe 1 NODE_LIST\n");
          analyse_passe_1(root->opr[0], type); // analyse de l'enfant NODE_DECLS
          analyse_passe_1(root->opr[1], type);  // analyse de l'enfant NODE_DECLS
          break;
        }
        
        case(NODE_FUNC):{
            printf("Analyse passe 1 NODE_FUNC\n");
            reset_env_current_offset();  // Reset de l'offset à 0

            ///// type de retour doit etre void:
            if(root->opr[0]->type != TYPE_VOID) {
                print_error(root->lineno, "type of main is not void");
                exit(1);
            }

            ////nom de la fonction doit etre main
            if(strcmp(root->opr[1]->ident, "main") != 0){
                 print_error(root->lineno, "invalid main name");
            }

            //// analyse du block 
            analyse_passe_1(root->opr[2], type);  // Analyse de l'enfant NODE_BLOCK

            ////////////
            // Mise à jour de l'offset 
            root->offset = get_env_current_offset();
           
            break; 
          }
        
        case(NODE_BLOCK):{
            printf("Analyse passe 1 NODE_BLOCK\n");
            push_context();              // Contexte local de la fonction (main)
            if (root->opr[0]!= NULL)
              analyse_passe_1(root->opr[0], type);

            if (root->opr[1]!= NULL)
              analyse_passe_1(root->opr[1], type);
            
            pop_context();               // Dépiler le libèrer le contexte courant
            break;

        }

        case(NODE_DECLS):{
          printf("Analyse passe 1 NODE_DECLS\n");
          node_type current_type;
          if (root->opr[0]->type == TYPE_VOID ){
            print_error(root->lineno, "variable declaration with type void");

          }
        
          if (root->opr[1]!= NULL)
              current_type = root->opr[0]->type;
              analyse_passe_1(root->opr[1], current_type); //analyse de l'enfant NODE_LIST ou NODE_DECL-
          break;

        }

        case(NODE_DECL):
        {
            printf("Analyse passe 1 NODE_DECL\n");

            // 1. mettre à jour le type de l'identifiant (hérité de NODE_DECLS)
            root->opr[0]->type = type;

            // 2. Ajouter la variable dans le contexte / environnement
            int32_t off = env_add_element(root->opr[0]->ident, root->opr[0]);
            if (off < 0) {
                print_error(root->lineno, "variable already declared in this scope");
            }
            root->opr[0]->offset = off; // Sauvegarde de l'offset dans le NODE_IDENT

            // 3. Analyser l'identifiant lui-même
            analyse_passe_1(root->opr[0], type);  

            // 4. Analyser l'expression d'initialisation si elle existe
            if (root->opr[1]) {
                analyse_passe_1(root->opr[1], type);  // l'initialisation doit être du même type
                // Vérification sémantique : type correspond au type de l'identifiant
                if (root->opr[1]->type != type) {
                    print_error(root->lineno, "type mismatch in variable initialization");
                }
            }
            break;
        }

        case(NODE_AFFECT):{
            printf("Analyse passe 1 NODE_AFFECT\n");
            analyse_passe_1(root->opr[0], type);
            analyse_passe_1(root->opr[1], type);

            if (root->opr[1]-> type != root->opr[0]-> type){
              print_error(root->lineno, "type mismatch");
            }

            break;
        }

        case(NODE_IDENT):
        {
<<<<<<< HEAD
          root->type =type ; 
            if (root->type == TYPE_NONE) {
=======
            root->type = type; // Mise à jour du type de la declaration 
            if(root->type == TYPE_NONE){ // Occurence d'utilisation (donc pas de type encore)
>>>>>>> 794d28e9183d9fd650f5d8635f6c2e253aef1e3d
                // C'est une utilisation, on cherche la déclaration dans l'environnement
                node_t decl_node = get_decl_node(root->ident);
                if (!decl_node) {
                    print_error(root->lineno, "variable used before declaration");
                }
                root->decl_node = decl_node;        // pointer vers la déclaration
                root->type = decl_node->type;       // copier le type de la déclaration
                root->offset = decl_node->offset;   // copier l'offset
            } else {
                // C'est une déclaration, on met à jour le type et l'offset
                root->type = type;
                // offset devrait déjà être assigné dans NODE_DECL
            }
            break;
        }

        case(NODE_TYPE):{
          printf("Analyse passe 1 NODE_TYPE\n");
          break;
        }

        case(NODE_INTVAL):{
          root->type = TYPE_INT;
          break;
        }

        case(NODE_BOOLVAL):{
          root->type = TYPE_BOOL;
          break;
        }

          case(NODE_STRINGVAL):{

          root -> offset = add_string(root->str);
          root -> type = TYPE_NONE;
          break;

        }
            
         /* -------- instructions de contrôle -------- */
        case NODE_IF: {
          printf("Analyse passe 1 NODE_IF\n");
          analyse_passe_1(root->opr[0], type); /* cond */
          if (root->opr[0]->type != TYPE_BOOL)
            print_error(root->lineno, "if condition must be bool");

          if (root->opr[1]) analyse_passe_1(root->opr[1], type); /* then */
          if (root->opr[2]) analyse_passe_1(root->opr[2], type); /* else */
            
          break;
        }

         
        case NODE_WHILE: {
          printf("Analyse passe 1 NODE_WHILE\n");
          if (root-> opr[0]) analyse_passe_1(root->opr[0], type);

          if (root->opr[0]->type != TYPE_BOOL)
                print_error(root->lineno, "while condition must be bool");
                
          if (root->opr[1]) analyse_passe_1(root->opr[1], type);

          break;

        }

        case NODE_FOR: {
          printf("Analyse passe 1 NODE_FOR\n");
          if (root->opr[0]) analyse_passe_1(root->opr[0], type); /* init */
          if (root->opr[1]) {
            analyse_passe_1(root->opr[1], type); /* cond */
            if (root->opr[1]->type != TYPE_BOOL)
                print_error(root->lineno, "for condition must be bool");
            }
            if (root->opr[2]) analyse_passe_1(root->opr[2], type); /* step */
            if (root->opr[3]) analyse_passe_1(root->opr[3], type); /* body */
            break;
        }


        case(NODE_DOWHILE):{
          printf("Analyse passe 1 NODE_DOWHILE\n");
          if (root->opr[0]) analyse_passe_1(root->opr[0], type); 
          if (root-> opr[1]) analyse_passe_1(root->opr[1], type);
            
          break;

        }

        case NODE_PRINT: {
          printf("Analyse passe 1 NODE_PRINT\n");
          if (root->opr[0]) analyse_passe_1(root->opr[0], type);
            break;

        }

        /* Opérateurs binaires (arithmétique, logique, etc.) */
        /* -------- opérations arithmétiques -------- */
        case NODE_PLUS:
        case NODE_MINUS:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD: {
            analyse_passe_1(root->opr[0], type);
            analyse_passe_1(root->opr[1], type);

            if (root->opr[0]->type != TYPE_INT || root->opr[1]->type != TYPE_INT)
                print_error(root->lineno, "arithmetic operator expects int operands");

            root->type = TYPE_INT;
            break;
        }
    

        /* -------- comparaisons -------- */
        case NODE_LT:
        case NODE_GT:
        case NODE_LE:
        case NODE_GE: {
            analyse_passe_1(root->opr[0], type);
            analyse_passe_1(root->opr[1], type);

            if (root->opr[0]->type != TYPE_INT || root->opr[1]->type != TYPE_INT)
                print_error(root->lineno, "comparison expects int operands");

            root->type = TYPE_BOOL;
            break;
        }
 
        case NODE_EQ:
        case NODE_NE: {
            analyse_passe_1(root->opr[0], type);
            analyse_passe_1(root->opr[1], type);

            if (root->opr[0]->type != root->opr[1]->type)
                print_error(root->lineno, "==/!= expects operands of same type");

            /* souvent: bool résultat */
            root->type = TYPE_BOOL;
            break;
        }

        /* -------- logique -------- */
        case NODE_AND:
        case NODE_OR: {
            analyse_passe_1(root->opr[0], type);
            analyse_passe_1(root->opr[1], type);

            if (root->opr[0]->type != TYPE_BOOL || root->opr[1]->type != TYPE_BOOL)
                print_error(root->lineno, "logical operator expects bool operands");

            root->type = TYPE_BOOL;
            break;
        }

        case NODE_NOT: {
            analyse_passe_1(root->opr[0], type);
            if (root->opr[0]->type != TYPE_BOOL)
                print_error(root->lineno, "! expects a bool operand");
            root->type = TYPE_BOOL;
            break;
        }

        case(NODE_BNOT):{
          if(root->opr[0]) analyse_passe_1(root->opr[0],type);
          if(root->opr[1])analyse_passe_1(root->opr[1],type);

          if (root->opr[0]->type != TYPE_INT)
            print_error(root->lineno,"bitwise not expects int operand");

        root->type = TYPE_INT;
        break;
        }

        /* -------- bitwise -------- */
      case(NODE_BAND):
      case(NODE_BOR):
      case(NODE_BXOR):{

        if (root->opr[0]->type != TYPE_INT || root->opr[1]->type != TYPE_INT)
          print_error(root->lineno,"bitwise operator expects int operands");

        root->type = TYPE_INT;
        break;
        }


        case(NODE_SLL):
        case(NODE_SRA):
        case(NODE_SRL):{
          if(root->opr[0]) analyse_passe_1(root->opr[0],type);
          if(root->opr[1])analyse_passe_1(root->opr[1],type);

          if (root->opr[0]->type != TYPE_INT || root->opr[1]->type != TYPE_INT)
            print_error(root->lineno,"shift operator expects int operands");

        root->type = TYPE_INT;
        break;

        }



    }

    //Parcours des enfants
    for (int i = 0; i < root->nops; i++) {
        analyse_passe_1(root->opr[i],type);
    }
    
}

  

