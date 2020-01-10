"""
解析注解RMI
"""
import re
import os
import string


class Param:
    """
    存储方法的参数类型
    """
    PATTERN_VAR = re.compile(r'[a-zA-Z0-9_]+')

    def __init__(self, ctx):
        self.content = ctx
        self.type = ""
        self.name = ""
        self.base_type = ""
        self.is_const = False
        self.is_point = False
        self.is_ref = False
        # self.is_forward = False

    pass

    def parser(self):
        # print(self.context)
        self.is_const = self.content.find("const") > -1
        self.is_point = self.content.find("*") > -1
        self.is_ref = self.content.find("&") > -1
        # self.is_forward = self.string.find("&&") > 0
        split = len(self.content)
        while split > 0:
            split -= 1
            if self.content[split] == ' ' or self.content[split] == '*' or self.content[split] == '&':
                break
        self.type = self.content[0: split + 1].strip()
        self.name = self.content[split + 1:].strip()
        res = re.findall(Param.PATTERN_VAR, self.type)
        self.base_type = res[len(res) - 1]
        return len(self.type) > 0 and len(self.name) > 0

    pass

    def __str__(self):
        return '[type:' + self.type + ', name:' + self.name + ', const:' + str(self.is_const) + ', point:' + str(
            self.is_point) \
               + ', ref:' + str(self.is_ref) + ']'

    pass


class RMIFunc:
    """
    存储RMI的方法类型
    """

    def __init__(self, ctx):
        ctx = ctx.strip()
        self._func_str = ctx
        self.name = ""
        self.ret_type = ""
        self.params = []

    pass

    def _parser_return_and_func_name(self, ctx):
        """
        提取返回值和函数名称
        """
        split = len(ctx)
        while split > 0:
            split -= 1
            if ctx[split] == ' ' or ctx[split] == '*' or ctx[split] == '&':
                break
        self.name = ctx[split + 1:].strip()
        self.ret_type = ctx[0: split + 1].strip()

    pass

    def _parser_param(self, ctx):
        """
        提取参数
        """
        if len(ctx) == 0:
            return True

        params = ctx.split(',')
        for p in params:
            res = Param(p.strip())
            if not res.parser():
                return False
            # print(res)
            self.params.append(res)
        # print(str(self.params))
        return True

    pass

    def parse(self):
        param_start_index = self._func_str.find('(')
        param_start_end = self._func_str.find(')')
        # print("%d-%d" % (param_start_index, param_start_end))
        ret_func_name = self._func_str[:param_start_index]
        self._parser_return_and_func_name(ret_func_name.strip())
        # print("-->"+ret_func_name)
        func_param = self._func_str[param_start_index + 1:param_start_end]
        # print("-->" + func_param)
        return self._parser_param(func_param)

    pass


class ModuleRmi:
    """
    解析某个模块的所有rmi function
    """
    __single_line_commit_pattern = re.compile(r'//(.*)', re.M)
    __multi_line_commit_pattern = re.compile(r'/\*(.*?)\*/', re.S)
    __func_line_break_pattern = re.compile(r'([\n]+)|([ ]{2,})', re.S)

    def __init__(self, _context, _name):
        self.name = _name
        self.cls_ctx = _context
        self.rmi_func = []

    pass

    def _clean_multi_line_comment(self):
        """
        去掉多行注释
        """
        self.cls_ctx = re.sub(self.__multi_line_commit_pattern, '', self.cls_ctx)

    pass

    def _clean_single_line_comment(self):
        """
        去掉C++的单行注释
        """
        self.cls_ctx = re.sub(self.__single_line_commit_pattern, '', self.cls_ctx)

    pass

    def _clean_empty_line(self):
        """
        去掉空行
        """
        lines = self.cls_ctx.split('\n')
        self.cls_ctx = ""
        for line in lines:
            line = line.strip()
            if line:
                self.cls_ctx += line + '\n'

    pass

    def _format_to_one_line(self):
        pattern = re.compile(r'(virtual (.+?);)|(Annotation\(@(.+?)\))', re.S)
        func = re.findall(pattern, self.cls_ctx)
        is_rmi_func = False
        for fun in func:
            if fun[0]:
                if not is_rmi_func:
                    continue
                # 只解析rmi function
                is_rmi_func = False
                line_clear = re.sub(self.__func_line_break_pattern, ' ', fun[1])
                # print(line_clear)
                rmi_function = RMIFunc(line_clear)
                if rmi_function.parse():
                    self.rmi_func.append(rmi_function)
            else:
                is_rmi_func = fun[2].find("@RMI") > -1

    pass

    def module_full_name(self):
        return 'I' + self.name + "Manager"

    pass

    def parser(self):
        self._clean_single_line_comment()
        self._clean_multi_line_comment()
        self._clean_empty_line()
        self._format_to_one_line()
    pass


class FileParser:

    def __init__(self, file):
        self.file_path = file
        self.file_context = ""
        self.rmi_module = []

    pass

    def parser(self):
        if not os.path.isfile(self.file_path):
            print("is not file!")
            return
        with open(self.file_path, "rt", encoding="utf-8", errors="ignore") as f:
            self.file_context = f.read()
            pattern = re.compile(r'class I(.+?)Manager(.+?)\};', re.S)
            res = re.findall(pattern, self.file_context)
            for r in res:
                m = ModuleRmi(r[1], r[0])
                m.parser()
                self.rmi_module.append(m)

    pass

    def rmi_modules(self):
        return self.rmi_module


class RmiFile:
    """
    生成rmi file
    """
    TEMPLATE_RMI_FUNCTION_ID = string.Template('__${module_name}_RMI_FUNC_${func_name}__')
    TEMPLATE_RMI_REQ_STRUCT = string.Template("""
template <>
class Rmi<${mod_name}>
{
public:
${rmi_func}
    Rmi(const svc_token_t& token)
        : m_svc_token(token)
    {
        mp_rpc_mng = dynamic_cast<IRPCManager*>(svc_find_manager(ManagerShareInfo<IRPCManager>::MNG_ID));
        AlwaysAssert(mp_rpc_mng);
    }
private:
    svc_token_t m_svc_token;
    IRPCManager* mp_rpc_mng;
};
    """)
    TEMPLATE_RMI_REP_STRUCT = string.Template("""
template <>
class RmiServerImpl<${mod_name}> : public IRmiServer
{
public:
    RmiServerImpl()
        : mp_mng(nullptr)
    { 
    }
    virtual int RmiExec(const int32_t& func_id, const std::string& args, std::string& res) override
    {
        switch (func_id) {
        ${rmi_case}
        default:
            LOG(ERROR) << "[rmi] unknown func id:" << func_id;
            break;
        }
        return 0;
    }
    void Init() override
    {
        mp_mng = dynamic_cast<${mod_name}*>(svc_find_manager(ManagerShareInfo<${mod_name}>::MNG_ID));
        AlwaysAssert(mp_mng);
    }
private:
${rmi_func}
private:
    ${mod_name}* mp_mng;
};
""")

    TABLE = "    "
    LISENCE = """#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

// 本文件由工具自动生成, 请勿修改

"""

    def __init__(self, _origin_file, _modules):
        self.origin_file = os.path.abspath(_origin_file)
        self.modules = _modules

    pass

    def _file_full_path(self):
        file_name = os.path.splitext(self.origin_file)[0]
        project_name = re.findall(r'(.+?)_interface', file_name)
        if len(project_name) != 1:
            print("file incorrect!")
            return None
        # print(project_name)
        return project_name[0] + "_rmi.h"

    @staticmethod
    def _to_func_id(mod_name, func_name):
        return RmiFile.TEMPLATE_RMI_FUNCTION_ID.substitute(module_name=mod_name, func_name=func_name)

    pass

    @staticmethod
    def _to_function_enum(module):
        s = 'enum\n{\n'
        for func in module.rmi_func:
            # print(module.name + ":" + func.name)
            s += RmiFile.TABLE + RmiFile._to_func_id(module.name, func.name) + ',\n'
        s += '};'
        return s

    pass

    @staticmethod
    def _to_req_function(module, func):
        s = ""
        # 构造函数名
        s += RmiFile.TABLE + func.ret_type + " "
        s += func.name + "("
        param_ser = ""
        param_str = ""
        first = True
        for p in func.params:
            if first:
                param_str += p.content
                first = False
            else:
                param_str += ", " + p.content
            param_ser += " << " + p.name

        s += param_str + ")\n" + RmiFile.TABLE + "{\n"
        # 构造序列化参数
        s += RmiFile.TABLE * 2 + "CBinaryStream stream;\n"
        s += RmiFile.TABLE * 2 + "stream" + param_ser + ";\n"
        s += RmiFile.TABLE * 2 + "std::string retData;\n"
        # 构造请求
        func_id = RmiFile._to_func_id(module.name, func.name)
        s += RmiFile.TABLE * 2 + "mp_rpc_mng->RmiCall(m_svc_token, ManagerShareInfo<" + module.module_full_name() + \
             ">::MNG_ID, " + func_id + ", stream.str(), retData);\n"

        # 构造反序列化
        ret_not_void = (func.ret_type != "void")
        if ret_not_void:
            s += RmiFile.TABLE * 2 + func.ret_type + " __return_val__ = { 0 };\n"
        s += RmiFile.TABLE * 2 + "if (is_in_co() && rmi_last_err() == RMI_CODE_OK) {\n"
        if ret_not_void or len(func.params):
            s += RmiFile.TABLE * 3 + "CBinaryStream retStream(retData.data(), retData.length());\n"
        der_str = RmiFile.TABLE * 3 + "retStream"
        need_der = False
        # 反序列化返回值
        if ret_not_void:
            der_str += " >> __return_val__"
            need_der = True
        # 反序列化引用参数
        for p in func.params:
            if p.is_point or p.is_ref:
                der_str += " >> " + p.name
                need_der = True
        if need_der:
            s += der_str + ";\n"
        s += RmiFile.TABLE * 2 + "}\n"
        if ret_not_void:
            s += RmiFile.TABLE * 2 + "return __return_val__;\n"
        s += RmiFile.TABLE + "}\n"
        return s

    @staticmethod
    def _to_rep_rmi_case(module, func):
        s = RmiFile.TABLE * 2 + "case " + RmiFile._to_func_id(module.name, func.name) + ":{\n"
        s += RmiFile.TABLE * 3 + func.name + "(args, res);\n"
        s += RmiFile.TABLE * 3 + "break;\n"
        s += RmiFile.TABLE * 2 + "}\n"
        return s

    pass

    @staticmethod
    def _to_rep_rmi_func(func):
        s = RmiFile.TABLE + "void " + func.name + "(const std::string& args, std::string& res)\n"
        s += RmiFile.TABLE + "{\n"
        empty_args = (len(func.params) == 0)
        s_req_param = ""
        s_param_ret = ""
        if not empty_args:
            s_param_der = ""
            s += RmiFile.TABLE * 2 + "CBinaryStream unpack_args(args.data(), args.size());\n"
            for p in func.params:
                s += RmiFile.TABLE * 2 + p.base_type + " " + p.name + " = { 0 };\n"
                s_param_der += " >> " + p.name
                if p.is_point or p.is_ref:
                    s_param_ret += " << " + p.name
                if p.is_point:
                    s_req_param += ",&" + p.name
                else:
                    s_req_param += "," + p.name
            s += RmiFile.TABLE * 2 + "unpack_args" + s_param_der + ";\n"

        ret_is_void = (func.ret_type == "void")

        if not ret_is_void or len(s_param_ret):
            s += RmiFile.TABLE * 2 + "CBinaryStream pack_res;\n"
        if ret_is_void:
            s += RmiFile.TABLE * 2 + "mp_mng->" + func.name + "(" + s_req_param[1:] + ");\n"
        else:
            s += RmiFile.TABLE * 2 + "pack_res << mp_mng->" + func.name + "(" + s_req_param[1:] + ");\n"
        if len(s_param_ret) > 0:
            s += RmiFile.TABLE * 2 + "pack_res" + s_param_ret + ";\n"
        s += RmiFile.TABLE * 2 + "res.append(pack_res.data(), pack_res.size());\n"
        s += RmiFile.TABLE + "}\n"
        return s

    pass

    @staticmethod
    def _to_module_str(mod):
        s_function_enum = RmiFile._to_function_enum(mod)
        s_req_function = ""
        s_rep_case = ""
        s_rep_function = ""
        for fun in mod.rmi_func:
            s_req_function += RmiFile._to_req_function(mod, fun) + "\n"
            s_rep_function += RmiFile._to_rep_rmi_func(fun) + "\n"
            s_rep_case += RmiFile._to_rep_rmi_case(mod, fun)
        s = s_function_enum
        s += RmiFile.TEMPLATE_RMI_REQ_STRUCT.substitute(mod_name=mod.module_full_name(), rmi_func=s_req_function)
        s += RmiFile.TEMPLATE_RMI_REP_STRUCT.substitute(mod_name=mod.module_full_name(), rmi_case=s_rep_case,
                                                        rmi_func=s_rep_function)
        return s

    pass

    def _content(self):
        s = RmiFile.LISENCE
        s += "#include \"" + os.path.basename(self.origin_file) + "\"" \
            "\n#include <core/module/rmi.h>\n"
        for module in self.modules:
            s += RmiFile._to_module_str(module)
        return s

    def generate(self):
        file_path = self._file_full_path()
        if file_path is None:
            return False
        print(file_path)
        with open(file_path, 'wt', encoding='utf-8') as f:
            print(self._content())
            f.write(self._content())

        return True


def annotation_parser_rmi(filepath):
    file_parser = FileParser(filepath)
    file_parser.parser()
    rmi_file = RmiFile(filepath, file_parser.rmi_modules())
    rmi_file.generate()
    return 0
