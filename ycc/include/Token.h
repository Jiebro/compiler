//
// Created by yk on 17-12-31.
//

#ifndef YCC_TOKEN_H
#define YCC_TOKEN_H

#include <cstdio>
#include <ctime>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <memory>
#include "enumerate.h"
class MacroPreprocessor;
namespace DataStruct {
    struct File{
        std::shared_ptr<std::ifstream> file;  // 文件流
        std::string p;     // 字符串流
        std::string name;  //文件名
        int cur;
        int line;
        int column;
        int ntok;     // 该流中有多少个token
        int last;     // 从流中读取的最后一个字符
        time_t mtime; // 最后修改时间
    } ;

    struct SourceLoc{
        char *file;
        int line;
    } ;

    struct Type {
        Type(DataStruct::TYPE_KIND k,int s=0,int a=0,bool ui=false):kind(k),size(s),align(a),usig(ui){}
        Type(DataStruct::TYPE_KIND k,bool is_struct=false):kind(k),is_struct(is_struct){}
        Type(DataStruct::TYPE_KIND k,int s,int a,std::shared_ptr<Type>& ptr,int len=0):kind(k),size(s),align(a),ptr(ptr),len(len){}
        Type(DataStruct::Type k,const std::shared_ptr<DataStruct::Type>&ret,const std::vector<DataStruct::Token>& params,bool hasva,
             bool oldstype):kind(k),rettype(ret),params(params),hasva(hasva),oldstyle(oldstype){}
        Type()= default;
        DataStruct::TYPE_KIND kind;
        int size;
        int align;
        bool usig; // 是否是unsigned
        bool isstatic;
        // pointer or array
        std::shared_ptr<Type> ptr= nullptr;
        // array length
        int len;
        // struct
        std::map<std::map<const char *, void *>, std::vector<void *>> *fields;
        int offset;
        bool is_struct; // true：struct, false： union
        // bitfield
        int bitoff;
        int bitsize;
        std::shared_ptr<Type> rettype;   //函数返回类型
        std::vector<Token> params;
        bool hasva;
        //http://en.cppreference.com/w/c/language/function_declaration
        bool oldstyle;
    };

    struct Token{
        TOKEN_TYPE kind;
        std::shared_ptr<File> file;
        int line=1;
        int column=1;
        bool space= false;   // 该token是否有个前导空格
        bool bol= false;     // 该token是否是一行的开头
        int count=0;    // token number in a file, counting from 0.
        std::set<std::string > hideset; // 宏展开
        // TKEYWORD
        AST_TYPE id;
        // TSTRING or TCHAR
        std::shared_ptr<std::string> sval;
        int c='\0';
        DataStruct::ENCODE enc=DataStruct::ENCODE::ENC_NONE;
        // TMACRO_PARAM
        bool is_vararg= false;   //是否是可变参数宏
        int position=-1;        //对于函数宏，代表该token是第几个参数
        Token()= default;

        Token(DataStruct::TOKEN_TYPE id,std::string str="",int c='\0',DataStruct::ENCODE enc=DataStruct::ENCODE::ENC_NONE):kind(id),sval(std::make_shared<std::string>(str)),c(c),enc(enc){}
        Token(DataStruct::TOKEN_TYPE id, bool is_var= false,int pos=-1):kind(id),is_vararg(is_var),position(pos){}
        Token(DataStruct::TOKEN_TYPE id,DataStruct::AST_TYPE id2):kind(id),id(id2){}
    };

    //条件宏的展开用的结构体：
    // ctx： 处于哪种条件宏中，如#if，#else，#elif等
    //include_guard:包含哨
    //file：存在包含哨的时候才可用，代表包含的文件
    //wastrue：该处代码块是否需要展开
    using CondIncl=struct {
        DataStruct::CondInclCtx ctx;
        std::string include_guard;
        std::shared_ptr<DataStruct::File> file;
        bool wastrue;
    } ;

    //宏展开相应结构体
    //kind：宏的类型，可选值为
    //    MACRO_OBJ:对象宏
    //    MACRO_FUNC：函数宏
    //    MACRO_SPECIAL：特殊宏，为c语言内置的宏
    //nargs：对于函数宏代表起参数数目
    //body：参数实体
    class Macro{
    public:
        Macro &operator=(const Macro&t) = default;
        Macro(const Macro&)= default;
        Macro()= default;
        Macro &operator=(Macro&&macro1)noexcept{
            kind=macro1.getKind();
            nargs=macro1.getNargs();
            is_varg=macro1.isIs_varg();
            fn=macro1.getFn();
            using itertype=decltype(macro1.getBody().begin());
            for (itertype beg=macro1.getBody().begin(),endg=macro1.getBody().end();beg!=endg;++beg)
                body.push_back(std::move(*beg));
        }
        Macro(Macro&&macro1)noexcept :kind(macro1.getKind()),nargs(macro1.getNargs()),is_varg(macro1.isIs_varg()),fn(macro1.getFn()){
            using itertype=decltype(macro1.getBody().begin());
            for (itertype beg=macro1.getBody().begin(),endg=macro1.getBody().end();beg!=endg;++beg)
                body.push_back(std::move(*beg));
        }
        Macro(DataStruct::MacroType t,std::vector<DataStruct::Token > body,bool is=false,int nargs=0):kind(t),body(body),is_varg(is),nargs(nargs){}
        Macro(DataStruct::MacroType t,std::function<void(MacroPreprocessor* ,const DataStruct::Token &)> f= nullptr):kind(t),fn(f){}

        DataStruct::MacroType getKind() const {
            return kind;
        }

        void setKind(DataStruct::MacroType kind) {
            this->kind = kind;
        }

        int getNargs() const {
            return nargs;
        }

        void setNargs(int nargs) {
            this->nargs = nargs;
        }

        const std::vector<DataStruct::Token > &getBody() const {
            return body;
        }

        void setBody(std::vector<DataStruct::Token> body) {
            this->body = body;
        }

        bool isIs_varg() const {
            return is_varg;
        }

        void setIs_varg(bool is_varg) {
            this->is_varg = is_varg;
        }

        const std::function<void(MacroPreprocessor* ,const DataStruct::Token &)> &getFn() const {
            return fn;
        }

        void setFn(const std::function<void(MacroPreprocessor* ,const DataStruct::Token &)> &fn) {
            this->fn = fn;
        }

    private:
        DataStruct::MacroType kind=DataStruct::MacroType::MACRO_INVALID;
        int nargs=0;
        std::vector<DataStruct::Token > body;
        bool is_varg= false;
        std::function<void (MacroPreprocessor* ,const DataStruct::Token&)> fn= nullptr;
    };
    class Node;
    struct BASE_Node{
//        NODETYPE kind;
//        BASE_Node(NODETYPE tok):kind(tok){}
        std::shared_ptr<Type> ty;
        std::shared_ptr<SourceLoc> sourceloc;
        virtual ~BASE_Node()=0;
    };

    struct CTL_Node: public BASE_Node{
//        CTL_Node():BASE_Node(NODETYPE::CIL){}
        long ival;
    };
    struct FD_Node:public BASE_Node{
//        FD_Node():BASE_Node(NODETYPE::FD){}
        double fval;
        std::string flabel;
    };
    struct STR_Node:public BASE_Node{
//        STR_Node():BASE_Node(NODETYPE::STR){}
        std::string sval;
        std::string slabel;
    };
    struct LGV_Node:public BASE_Node{
//        LGV_Node():BASE_Node(NODETYPE::LGV){}
        std::string varname;
        // local
        int loff;
        std::vector<DataStruct::Token> lvarinit;
        // global
        std::string glabel;
    };
    struct BIOP_Node:public BASE_Node{
//        BIOP_Node():BASE_Node(NODETYPE::BIOP){}
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
    };
    struct RET_Node:public BASE_Node{
//        RET_Node():BASE_Node(NODETYPE::RET){}
        std::shared_ptr<Node> retval;
    };
    struct FCFD_Node:public BASE_Node{
//        FCFD_Node():BASE_Node(NODETYPE::FCFD){}
        std::string fname;
        // Function call
        std::vector<Token> args;
        std::shared_ptr<Type> ftype;
        // Function pointer or function designator
        std::shared_ptr<Node> fptr;
        // Function declaration
        std::vector<Token> params;
        std::vector<Token> localvars;
        std::shared_ptr<Node> body;
    };
    struct DEC_Node:public BASE_Node{
//        DEC_Node():BASE_Node(NODETYPE::DEC){}
        std::shared_ptr<Node> declvar;
        std::vector<Token> declinit;
    };
    struct INIT_Node:public BASE_Node{
//        INIT_Node():BASE_Node(NODETYPE::INIT){}
        std::shared_ptr<Node> initval;
        int initoff;
        std::shared_ptr<Type> totype;
    };
    struct IFTOP_Node:public BASE_Node{
//        IFTOP_Node():BASE_Node(NODETYPE::IFTOP){}
        std::shared_ptr<Node> cond;
        std::shared_ptr<Node> then;
        std::shared_ptr<Node> els;
    };
    struct STRREF_Node:public BASE_Node{
//        STRREF_Node():BASE_Node(NODETYPE::STRREF){}
        std::shared_ptr<Node> struc;
        std::string field;
        std::shared_ptr<Type> fieldtype;
    };
    struct COMPO_Node:public BASE_Node{
//        COMPO_Node():BASE_Node(NODETYPE::COMPO){}
        std::vector<DataStruct::Token> stmts;
    };
    struct GOLA_Node:public BASE_Node{
//        GOLA_Node():BASE_Node(NODETYPE::GOLA){}
        std::string label;
        std::string newlabel;
    };
    struct UNOP_Node:public BASE_Node{
//        UNOP_Node():BASE_Node(NODETYPE::UNOP){}
        std::shared_ptr<Node> unop;
    };


    class Node{
    public:
//        using Fd =struct {
//            double fval;
//            std::string flabel;
//        }; // float or double
//        using Str=struct {
//            std::string sval;
//            std::string slabel;
//        };// string
//        using LGv=struct {
//            std::string varname;
//            // local
//            int loff;
//            std::vector<DataStruct::Token> lvarinit;
//            // global
//            std::string glabel;
//        };            // local/global 变量
//        using Biop=struct {
//            std::shared_ptr<Node> left;
//            std::shared_ptr<Node> right;
//        };      // binary op
//        using Fcfd=struct {
//            std::string fname;
//            // Function call
//            std::vector<Token> args;
//            std::shared_ptr<Type> ftype;
//            // Function pointer or function designator
//            std::shared_ptr<Node> fptr;
//            // Function declaration
//            std::vector<Token> params;
//            std::vector<Token> localvars;
//            std::shared_ptr<Node> body;
//        };// 函数调用或声明
//        using Dec=struct {
//            std::shared_ptr<Node> declvar;
//            std::vector<Token> declinit;
//        };    // 声明
//        using Init=struct {
//            std::shared_ptr<Node> initval;
//            int initoff;
//            std::shared_ptr<Type> totype;
//        };  // 初始化
//        using Iftop=struct {
//            std::shared_ptr<Node> cond;
//            std::shared_ptr<Node> then;
//            std::shared_ptr<Node> els;
//        };// if语句或ternary op
//        using Gola=struct {
//            std::string label;
//            std::string newlabel;
//        };// goto label
//        using Strref=struct {
//            std::shared_ptr<Node> struc;
//            std::string field;
//            std::shared_ptr<Type> fieldtype;
//        };// struct引用
        Node():tok(NODETYPE::CIL),ival(0){}
        Node(const Node&r):tok(r.tok){copyUnion(r);}
        ~Node(){clear();}
        Node&operator=(const Node& t);
        static Node make_CIL_node(long);
        static Node make_FD_node(double, const std::string&);
        static Node make_STR_node(const std::string&, const std::string&);
        static Node make_LGV_node(const std::string&,int , const std::vector<Token>&, const std::string&);
        static Node make_BIOP_node(const std::shared_ptr<Node>&, const std::shared_ptr<Node>& );
        static Node make_RET_node(const std::shared_ptr<Node>&);
        static Node make_FCFD_node(const std::string&, const std::vector<Token>&,const std::shared_ptr<Type>&, std::shared_ptr<Node>&,const std::vector<Token>&,
                const std::vector<Token>&, const std::shared_ptr<Node>&);
        static Node make_DEC_node(const std::shared_ptr<Node>&, const std::vector<Token>&);
        static Node make_INIT_node(const std::shared_ptr<Node>&,int, const std::shared_ptr<Type>&);
        static Node make_IFTOP_node(const std::shared_ptr<Node>&, const std::shared_ptr<Node>&, const std::shared_ptr<Node>&);
        static Node make_STRREF_node(const std::shared_ptr<Node>&, const std::string&, const std::shared_ptr<Type>&);
        static Node make_COMPO_node(const std::vector<Token>&);
        static Node make_GOLA_node(const std::string&, const std::string&);
        static Node make_UNOP_node(const std::shared_ptr<Node>&);


        long getIval() const;

        void setIval(long);

        AST_TYPE getKind() const;

        void setKind(AST_TYPE);

        const std::shared_ptr<Type> &getTy() const;

        void setTy(const std::shared_ptr<Type> &ty);

        const std::shared_ptr<SourceLoc> &getSourceloc() const;

        void setSourceloc(const std::shared_ptr<SourceLoc> &sourceloc);


//        union {
            // char int or long
            long ival;
            // return 语句
            std::shared_ptr<Node> retval;
            // compound 语句
            std::vector<DataStruct::Token> stmts;
//            std::shared_ptr<Fd> fd;
//            std::shared_ptr<Str> str;
//            std::shared_ptr<LGv> lgv;
//            std::shared_ptr<Biop> biop;
//            std::shared_ptr<Node> unop;
//            std::shared_ptr<Fcfd> fcfd;
//            std::shared_ptr<Dec> dec;
//            std::shared_ptr<Init> init;
//            std::shared_ptr<Iftop> iftop;
//            std::shared_ptr<Gola> gola;
//            std::shared_ptr<Strref> strref;

        std::shared_ptr<FD_Node> fd;
        std::shared_ptr<STR_Node> str;
        std::shared_ptr<LGV_Node> lgv;
        std::shared_ptr<BIOP_Node> biop;
        std::shared_ptr<Node> unop;
        std::shared_ptr<FCFD_Node> fcfd;
        std::shared_ptr<DEC_Node> dec;
        std::shared_ptr<INIT_Node> init;
        std::shared_ptr<IFTOP_Node> iftop;
        std::shared_ptr<GOLA_Node> gola;
        std::shared_ptr<STRREF_Node> strref;
//        };
    private:
        AST_TYPE kind;
        std::shared_ptr<Type> ty;
        std::shared_ptr<SourceLoc> sourceloc;
        NODETYPE tok;
        Node& copyUnion(const Node& r);
        void clear();
    };
//    typedef struct Node {
//        int kind;
//        Type *ty;
//        SourceLoc *sourceLoc;
//        union {
//            // Char, int, or long
//            long ival;
//            // Float or double
//            struct {
//                double fval;
//                char *flabel;
//            };
//            // String
//            struct {
//                char *sval;
//                char *slabel;
//            };
//            // Local/global variable
//            struct {
//                char *varname;
//                // local
//                int loff;
//                Vector *lvarinit;
//                // global
//                char *glabel;
//            };
//            // Binary operator
//            struct {
//                struct Node *left;
//                struct Node *right;
//            };
//            // Unary operator
//            struct {
//                struct Node *operand;
//            };
//            // Function call or function declaration
//            struct {
//                char *fname;
//                // Function call
//                Vector *args;
//                struct Type *ftype;
//                // Function pointer or function designator
//                struct Node *fptr;
//                // Function declaration
//                Vector *params;
//                Vector *localvars;
//                struct Node *body;
//            };
//            // Declaration
//            struct {
//                struct Node *declvar;
//                Vector *declinit;
//            };
//            // Initializer
//            struct {
//                struct Node *initval;
//                int initoff;
//                Type *totype;
//            };
//            // If statement or ternary operator
//            struct {
//                struct Node *cond;
//                struct Node *then;
//                struct Node *els;
//            };
//            // Goto and label
//            struct {
//                char *label;
//                char *newlabel;
//            };
//            // Return statement
//            struct Node *retval;
//            // Compound statement
//            Vector *stmts;
//            // Struct reference
//            struct {
//                struct Node *struc;
//                char *field;
//                Type *fieldtype;
//            };
//        };
//    } Node;
}
#endif //YCC_TOKEN_H
