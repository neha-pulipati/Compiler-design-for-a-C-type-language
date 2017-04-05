#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <string>

extern "C"{
#include "compiler.h"
}

using namespace std;

vector<string> variables;
map<string, struct ValueNode*> variableMap;
struct ValueNode* search_var_map(string);
void assign_value(struct ValueNode*);
void parse_variables();
struct StatementNode* parse_body();
struct StatementNode* parse_stmt();
struct StatementNode* parse_stmt_list();
struct StatementNode* case_list(string, struct StatementNode*);
struct StatementNode* parse_case(string, struct StatementNode*);

struct ValueNode* search_var_map(string item) {
    return variableMap[item];
}


struct StatementNode* parse_generate_intermediate_representation(){
    struct StatementNode* st;
    parse_variables();
    st = parse_body();
    return st;
}

void parse_variables(){
    ttype = getToken();
    while(ttype != SEMICOLON){
        if(ttype == ID) {
            variables.push_back(string(token));
            struct ValueNode* temp;
            temp = new ValueNode();
            temp->name = token;
            temp->value = 0;
            //pair<string, struct ValueNode*> insert_pair = {string(token), temp};
            variableMap.insert(make_pair(string(token), temp));
        }
        ttype = getToken();
    }
}

struct StatementNode* parse_body() {
    struct StatementNode *stl;

    ttype = getToken();
    if (ttype == LBRACE) {
        stl = parse_stmt_list();
        ttype = getToken();
        if (ttype == RBRACE) {
            return stl;
        }
    }
}


struct StatementNode* parse_stmt_list(){
    struct StatementNode* st;
    struct StatementNode* stl;
    st = parse_stmt();
    ttype = getToken();
    if(ttype == IF || ttype == WHILE || ttype == SWITCH || ttype == ID || ttype == PRINT){
        ungetToken();
        stl = parse_stmt_list();
        if(st->type == IF_STMT){
            st->next->next = stl;
        } else{
            st->next = stl;
        }
        return st;
    } else{
        ungetToken();
        return st;
    }
}

struct StatementNode* parse_case(string item, struct StatementNode* noop){
    struct StatementNode* st = new StatementNode();

    st->type = IF_STMT;
    struct IfStatement *ifStatementNode = new IfStatement();
    st->if_stmt = ifStatementNode;
    ttype = getToken(); //case
    ttype = getToken(); //num
    ifStatementNode->condition_operand1 = search_var_map(item);

    ifStatementNode->condition_operand2 = new ValueNode();
    ifStatementNode->condition_operand2->value = atoi(token);

    getToken(); //colon
    ifStatementNode->condition_op = NOTEQUAL;

    ifStatementNode->false_branch = parse_body();

    struct StatementNode* temp = ifStatementNode->false_branch;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = noop;

    st->next = noop;

    ifStatementNode->true_branch = noop;

    return st;

}


struct StatementNode* case_list(string item, struct StatementNode* noop){
    struct StatementNode* st = new StatementNode();
    struct StatementNode* stl = new StatementNode();
    //getToken();
    st = parse_case(item, noop);
    getToken();
    if(ttype == CASE) {
        ungetToken();
        stl = case_list(item,noop);
        if(st->type == IF_STMT){
            st->if_stmt->true_branch = stl;
        }
        else{
            st->next = stl;
        }
        return st;
    }

    else if(ttype == DEFAULT){
        getToken();
        struct StatementNode* temp=st;
       //temp = st;
        //temp = parse_body();

        while (temp->next->next != NULL) {
            temp = temp->next->next;
        }

        temp->if_stmt->true_branch = parse_body();
        struct StatementNode* store = temp->if_stmt->true_branch;

        while(store->next!=NULL)
            store=store->next;

        store->next=noop;
        return st;

//            temp->if_stmt->true_branch = parse_body();
//            while (temp->if_stmt->true_branch != NULL){
//                temp->if_stmt->true_branch = temp->if_stmt->true_branch->next;
//            }
//            temp->if_stmt->true_branch->next = noop;
    }

    else{
        ungetToken();
        return st;
    }

}

struct StatementNode* parse_stmt() {
    ttype = getToken();
    if (ttype == SWITCH) {
        struct StatementNode *st=new StatementNode();
        ttype = getToken();
        string variable = string(token);
        ttype = getToken();
        if (ttype == LBRACE) {
            struct StatementNode* noop = new StatementNode();
            noop->type = NOOP_STMT;
            st = case_list(variable, noop);
            ttype = getToken();
            if(ttype == RBRACE){
                return st;
            }
        }
    }
    else if (ttype == WHILE) {
        struct StatementNode *st = new StatementNode();
        st->type = IF_STMT;
        struct IfStatement *ifStatementNode = new IfStatement();
        st->if_stmt = ifStatementNode;
        ttype = getToken();
        if (ttype == ID) {
            ifStatementNode->condition_operand1 = search_var_map(string(token));
        }
        if (ttype == NUM) {
            ifStatementNode->condition_operand1 = new ValueNode();
            ifStatementNode->condition_operand1->value = atoi(token);
        }
        ttype = getToken();
        ifStatementNode->condition_op = ttype;
        ttype = getToken();
        if (ttype == ID) {
            ifStatementNode->condition_operand2 = search_var_map(string(token));
        }
        if (ttype == NUM) {
            ifStatementNode->condition_operand2 = new ValueNode();
            ifStatementNode->condition_operand2->value = atoi(token);
        }

        ifStatementNode->true_branch = parse_body();

        struct StatementNode *gt = new StatementNode();
        gt->type = GOTO_STMT;
        struct GotoStatement* structGotoStatement = new GotoStatement();
        gt->goto_stmt = structGotoStatement;
        structGotoStatement->target = st;


        if (ifStatementNode->true_branch == NULL) {
            ifStatementNode->true_branch = gt;
        }

        struct StatementNode *temp;
        temp = ifStatementNode->true_branch;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = gt;

        struct StatementNode *noop = new StatementNode();
        noop->type = NOOP_STMT;
        ifStatementNode->false_branch = noop;
        st->next = noop;

        return st;
    }
    else if (ttype == IF) {
        struct StatementNode *st = new StatementNode();
        st->type = IF_STMT;
        struct IfStatement *ifStatementNode = new IfStatement();
        st->if_stmt = ifStatementNode;
        ttype = getToken();
        if (ttype == ID) {
            ifStatementNode->condition_operand1 = search_var_map(string(token));
        }
        if (ttype == NUM) {
            ifStatementNode->condition_operand1 = new ValueNode();
            ifStatementNode->condition_operand1->value = atoi(token);
        }
        ttype = getToken();
        ifStatementNode->condition_op = ttype;
        ttype = getToken();
        if (ttype == ID) {
            ifStatementNode->condition_operand2 = search_var_map(string(token));
        }
        if (ttype == NUM) {
            ifStatementNode->condition_operand2 = new ValueNode();
            ifStatementNode->condition_operand2->value = atoi(token);
        }

        ifStatementNode->true_branch = parse_body();

        struct StatementNode *noop = new StatementNode();
        noop->type = NOOP_STMT;

        if (ifStatementNode->true_branch == NULL) {
            ifStatementNode->true_branch = noop;
        }

        struct StatementNode *temp;
        temp = ifStatementNode->true_branch;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = noop;

        ifStatementNode->false_branch = noop;
        st->next = noop;

        return st;
    }
    else if (ttype == ID) {
        struct StatementNode *st = new StatementNode();
        st->type = ASSIGN_STMT;
       // struct AssignmentStatement *assignmentStatementNode;
        st->assign_stmt = new AssignmentStatement();
        string leftHandSide = string(token);
        ttype = getToken();
        if (ttype == EQUAL) {
            getToken();
            if(ttype == NUM) {
                st->assign_stmt->operand1 = new ValueNode();
                st->assign_stmt->operand1->value = atoi(token);
            }
            if (ttype == ID)
                st->assign_stmt->operand1 = search_var_map(string(token));
            //if valueNode is null, create and update the map
            getToken();
            if (ttype == SEMICOLON) {
                st->assign_stmt->operand2 = NULL;
                st->assign_stmt->op = 0;
            } else {
                st->assign_stmt->op = ttype;
                ttype = getToken();
                if (ttype == ID) {
                    st->assign_stmt->operand2 = search_var_map(string(token));
                }
                if (ttype == NUM) {
                    st->assign_stmt->operand2 = new ValueNode();
                    st->assign_stmt->operand2->value = atoi(token);
                }
                getToken();
            }
        }
        st->assign_stmt->left_hand_side = search_var_map(leftHandSide);
        return st;
    }
    else if (ttype == PRINT) {
        struct StatementNode *st = new StatementNode();
        st->type = PRINT_STMT;
       // struct PrintStatement *printStatementNode;
        st->print_stmt = new PrintStatement();
        getToken();
        st->print_stmt->id = search_var_map(string(token));
        getToken();
        return st;
    }
    else{
        return NULL;
    }
}