#include <jsgf/jsgf.h>
#include <jsgf/jsgf_internal.h>
#include <wfst/rule.h>
#include <util/timer.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

static void display_grammar(jsgf_t* grammar){
    std::cout << "=========== grammar " << grammar->name << " ===========" << std::endl;
    hash_table_t *rules = grammar->rules;
    jsgf_rule_iter_t *itor = NULL;
    for (itor = jsgf_rule_iter(grammar); itor; itor = jsgf_rule_iter_next(itor)) {
        const char * key = itor->ent->key;
        jsgf_rule_t *rule = jsgf_rule_iter_rule(itor);
        jsgf_rhs_t *rhs;
        std::ostringstream rule_ostr; 
        rule_ostr << " " << rule->name << " = ";
        //LOG_DEBUG("rule = %s ", rule->name);
        for (rhs = rule->rhs; rhs; rhs = rhs->alt) {
            //LOG_DEBUG(">>> rhs");
            if(rhs != rule->rhs){
                rule_ostr << " | " ;
            }    
            gnode_t *gn;
            for (gn = rhs->atoms; gn; gn = gnode_next(gn)) {
                jsgf_atom_t *atom = (jsgf_atom_t *)gnode_ptr(gn);
                //LOG_DEBUG(">>>>>> atom = %s ", atom->name);
                if(gn != rhs->atoms){
                    rule_ostr << " , " ;
                }
                if(jsgf_atom_is_rule(atom)){
                    rule_ostr << " $" << atom->name;
                }else{
                    rule_ostr << " " << atom->name;
                }
                if(glist_count(atom->tags)>0){
                    rule_ostr << " @ ";
                }
                gnode_t *gn_tag;
                for (gn_tag = atom->tags; gn_tag; gn_tag = gnode_next(gn_tag)) {
                    const char* tag_str = static_cast<const char*>(gn_tag->data.ptr);
                    //LOG_DEBUG(">>>>>>>>> tag = %s ", tag_str);
                    if(gn_tag != atom->tags){
                        rule_ostr << " $ " ;
                    }
                    rule_ostr << tag_str;
                }
            }
        }
        //LOG_DEBUG("");
        //LOG_DEBUG("%s", rule_ostr.str().c_str());
        std::cout << rule_ostr.str() << std::endl;
    }
    std::cout << "==================================" << std::endl;
}

int main(int argc, char **argv)
{

    std::unordered_map<std::pair<std::string,std::string>, std::string, sogou::pair_hash>  m_complexRules;
    m_complexRules.insert(std::make_pair( std::make_pair("a","b") , "c" ));
    m_complexRules.insert(std::make_pair( std::make_pair("a1","b1") , "c1" ));

    std::pair<std::string,std::string> key("a","b");
    std::unordered_map<std::pair<std::string,std::string>, std::string, sogou::pair_hash>::iterator it;
    it = m_complexRules.find(key);
    if(it != m_complexRules.end()){
        std::string value = it->second;
        std::cout << "found a,b = " << value << std::endl;
    }else{
        std::cout << "not found a,b = " << std::endl;
    }
    key.first = "b";
    key.second = "c";
    it = m_complexRules.find(key);
    if(it != m_complexRules.end()){
        std::string value = it->second;
        std::cout << "found b,c = " << value << std::endl;
    }else{
        std::cout << "not found b,c = " << std::endl;
    }

    sogou::SymbolType type = 2342341324;
    sogou::ExpUnit  expUnit;
    sogou::pair_hash h_obj;
    auto a = h_obj(std::make_pair(type,&expUnit));
    auto b = h_obj(std::make_pair(type,&expUnit));
    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    if(a==b){
        std::cout << "equals " << std::endl;
    }

    if(argc<2){
        return 0;
    }
    char* filename_c = argv[1];

    sogou::RuleMgr ruleMgr(filename_c);
    if(ruleMgr.init()){
        //ruleMgr.displayJsgfRules();
        //ruleMgr.displayRules();
    }else{
        std::cout << "Parse failed!!!!!!!!!!!!!!!!!!" << std::endl;
    }

    return 0;
}

