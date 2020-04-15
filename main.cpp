#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include <set>
#include <memory>

using std::vector;
using std::unordered_map;
using std::string;
using std::deque;
using std::set;
using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;

class AddressBook{
 public:
    string phone_by_secondname(string &secondname){
        return phone_by_secondname_[secondname];
    }
    void add_contact(string& number, string& secondname){
        secondnames_.push_back(secondname);
        string* secondname_ptr = &(secondnames_[secondnames_.size()-1]);
        phone_by_secondname_[secondname] = number;
        secondname_by_phone_[number] = secondname;
        DigitNode* current_node = root;
        for(char digit:number){
            current_node->add_secondname(secondname_ptr);
            current_node = current_node->additive_step_by_char(digit);
        }
        current_node->add_secondname(secondname_ptr);
    }
    const vector<string*>& get_secondname_by_prefix(string &num){
        DigitNode* current_node = root;
        for(auto digit:num){
            current_node=current_node->step_by_char(digit);
            if(current_node== nullptr){
                return default_empty_;
            }
        }
        return current_node->get_secondnames();
    }
    vector<string*> stupid_search_by_pattern(string& pattern){
        vector<DigitNode*> condidates;
        condidates.push_back(root);
        for(char digit:pattern){
            vector<DigitNode*> temp_condidates;
            if(digit>='0' && digit<='9'){
                for(DigitNode* current:condidates){
                    if(current->step_by_char(digit)!= nullptr){
                        temp_condidates.push_back(current->step_by_char(digit));
                    }
                }
            }else{
                for(DigitNode* current:condidates) {
                    for(DigitNode* child:current->next_nodes){
                        if(child!= nullptr) {
                            temp_condidates.push_back(child);
                        }
                    }
                }
                }
            condidates = std::move(temp_condidates);
        }
        vector<string*> answer;
        for(DigitNode* temp_node:condidates){
            for(string* family:temp_node->get_secondnames()){
                answer.push_back(family);
            }
        }
        return answer;
    }
    ~AddressBook(){
        delete root;
    }
 private:
    struct DigitNode{
        //для оптимизации можно хранить в нодах указатели на строки с именами.
        //а все имена хранить в отдельных деках
        //(в них не инвалидируются указатели
     public:
        DigitNode* step_by_char(char c){
            return next_nodes[c-'0'];
        }
        DigitNode* additive_step_by_char(char c){
               int num = c-'0';
               if(next_nodes[num] == nullptr){
                   next_nodes[num] = new DigitNode();
               }
            return next_nodes[num];
        }
        const vector<string*>& get_secondnames(){
            return condidates;
        }
        void add_secondname(string* new_secondname){
         condidates.push_back(new_secondname);
     }
     //для отсутствия такоко костыля можно хранить детей в shred_ptr
     // и да, надо следовать правилу 5, но времена такие, времени нет
        ~DigitNode(){
            for(DigitNode* childs:next_nodes){
                delete childs;
            }
        }

     private:
        vector<DigitNode*> next_nodes = vector<DigitNode*>(10, nullptr);
        vector<string*> condidates;
        friend AddressBook;
    };

    DigitNode* root = new DigitNode;
    const vector<string*> default_empty_;
    unordered_map<string,string> phone_by_secondname_;
    unordered_map<string,string> secondname_by_phone_;
    deque<string> secondnames_;
};

int main() {
    AddressBook app;
    while(true){
        cout<<"1 to add contact\n"<< "2 for to search by family\n" << "3 to search by part of number\n"
        <<"4 to search by pattern\n" << "5 to exit\n";
        int option;
        cin>>option;
        if(option==1){
            string temp_phone;
            string temp_family;
            cout<<"family:";
            cin>>temp_family;
            cout<<"\nnumber:";
            cin>>temp_phone;
            cout<<"\n";
            app.add_contact(temp_phone,temp_family);
        }
        if(option==2){
            string temp_family;
            cout<<"family:";
            cin>>temp_family;
            cout<<'\n';
            cout<< app.phone_by_secondname(temp_family)<<'\n';
        }
        if(option==3){
            string temp_phone;
            cout<<"phone:";
            cin>>temp_phone;
            cout<<'\n';
            const vector<string*> ans = app.get_secondname_by_prefix(temp_phone);
            if(ans.empty()){
                cout<<"No users found\n";
            }else{
                for(string* probably_secondname:ans){
                    cout<<*probably_secondname<<'\n';
                }
            }
        }
        if(option==4){
            string pattern;
            cout<<"pattern:";
            cin>>pattern;
            vector<string*> ans = app.stupid_search_by_pattern(pattern);
            if(ans.empty()){
                cout<<"No users found\n";
            } else{
                for(string* probably_secondname:ans){
                    cout<<*probably_secondname<<'\n';
                }
            }
        }
        if(option==5){
            cout<<"thanks for use!";
            break;
        }

    }
    return 0;
}