//
// Created by yk on 18-2-18.
//

#ifndef YCC_FUNCTEST_H
#define YCC_FUNCTEST_H

#include <string>

namespace Test{
    void lexTokenTest(std::string);
    void newLineTest();
    void macroExpandTest(const std::string&,int);
    void NodeTest();
    void parserTest(const std::string&,int);
    void parserAttrTest(const std::string&path);
    void formatfile(const std::string&path,const std::string&content);
    void cformatfile(char *name,char *fmt,...);
    void emittest();
}
#endif //YCC_FUNCTEST_H
