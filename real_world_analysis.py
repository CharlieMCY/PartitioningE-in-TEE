import os
import sys
import time


def params_type():
    funcs = {}
    types = {}

    codeql_run = "codeql query run ..//..//query//host.ql " + \
            "--database=tee_example > " + \
            "out//params_type.out"
    os.system(codeql_run)

    if os.path.exists("out//params_type.out"):
        f = open("out//params_type.out", "r")
        all_lines = f.readlines()
        for params_types in all_lines[2:]:
            tmp = params_types.replace(" ", "").split('|')
            funcs[tmp[1]] = int(tmp[2])
    
    for key, params in funcs.items():
        types[key] = []
        types[key].append(params & 0x000f)
        types[key].append((params & 0x00f0) >> 4)
        types[key].append((params & 0x0f00) >> 8)
        types[key].append((params & 0xf000) >> 12)
    
    return types


def switch_query():
    funcs = {}
    codeql_run = "codeql query run ..//..//query//switch.ql " + \
            "--database=tee_example > " + \
            "out//func_id.out"
    os.system(codeql_run)

    if os.path.exists("out//func_id.out"):
        f = open("out//func_id.out", "r")
        all_lines = f.readlines()
        for fname in all_lines[2:]:
            tmp = fname.replace(" ", "").split('|')
            funcs[tmp[2]] = tmp[1]
    
    return funcs


def arrayaccess_query():
    alist = []
    codeql_run = "codeql query run ..//..//query//arrayaccess.ql " + \
            "--database=tee_example > " + \
            "out//arrayaccess.out"
    os.system(codeql_run)

    if os.path.exists("out//arrayaccess.out"):
        f = open("out//arrayaccess.out", "r")
        all_lines = f.readlines()
        for fname in all_lines[2:]:
            tmp = fname.replace(" ", "").split('|')
            alist.append(tmp[6])
    
    return alist


def if_query():
    iflist = []
    codeql_run = "codeql query run ..//..//query//ifstmt.ql " + \
            "--database=tee_example > " + \
            "out//ifstmt.out"
    os.system(codeql_run)

    if os.path.exists("out//ifstmt.out"):
        f = open("out//ifstmt.out", "r")
        all_lines = f.readlines()
        for fname in all_lines[2:]:
            tmp = fname.replace(" ", "").split('|')
            iflist.append(tmp[1])
    
    return iflist


def dataflow_query(types, funcs, alist, iflist):
    params_input = {}
    params_shared = {}
    current_func = ''

    codeql_run = "codeql query run ..//..//query//dataflow.ql " + \
        "--database=tee_example > " + \
        "out//dataflow.out"
    os.system(codeql_run)

    if os.path.exists("out//dataflow.out"):
        f = open("out//dataflow.out", "r")
        all_lines = f.readlines()
        for params_flow in all_lines[2:]:
            tmp = params_flow.replace(" ", "").split('|')
            if funcs.has_key('callto' + tmp[5]):
                current_func = funcs['callto' + tmp[5]]
            key = tmp[2] + '_' + tmp[1] + '_' + current_func
            if types.has_key(current_func):
                if not tmp[1].isdigit():
                    continue
                param_type = types[current_func][int(tmp[1])]
                
                if tmp[6] in alist:
                    tmp[4] = tmp[4] + 'accesstoarray'
                if tmp[6] in iflist:
                    tmp[4] = tmp[4] + 'if'

                if param_type >= 12 and param_type <= 15:
                    if params_shared.has_key(key):
                        tmp[6] = tmp[6].replace('-', '')
                        params_shared[key].append(tmp[4] + tmp[6])
                    else:
                        params_shared[key] = []
                        tmp[6] = tmp[6].replace('-', '')
                        params_shared[key].append(tmp[4] + tmp[6])
                else:
                    key = tmp[1] + '_' + current_func
                    item = tmp[4] + tmp[6]
                    if params_input.has_key(key):
                        if not item in params_input[key]:
                            params_input[key].append(item)
                    else:
                        params_input[key] = []
                        params_input[key].append(item)

    return params_input, params_shared


def unencrypt_query():
    count = 0
    ue_list = []
    params = {}
    codeql_run = "codeql query run ..//..//query//memory.ql " + \
        "--database=tee_example > " + \
        "out//unencrypt.out"
    os.system(codeql_run)

    if os.path.exists("out//unencrypt.out"):
        f = open("out//unencrypt.out", "r")
        all_lines = f.readlines()
        for params_flow in all_lines[2:]:
            tmp = params_flow.replace(" ", "").split('|')
            key = tmp[1]

            if params.has_key(key):
                params[key] = params[key] + tmp[2]
            else:
                params[key] = tmp[2]
            
            if 'snprintf' in tmp[2] or 'TEE_MemMove' in tmp[2] or '=' in tmp[2]:
                if not ('aes' in params[key] or 'enc' in params[key]):
                    loca = tmp[3].split(':')
                    num = int(loca[len(loca) - 2])
                    if num < 195:
                        count = count + 1
                        ue_list.append(tmp[3])
    
    return count, ue_list

def analysis(params_input, params_shared):
    count_i = 0
    count_s = 0

    ilist = []
    slist = []
    l_secmem = []

    for key in params_input:
        check = ''
        for item in params_input[key]:
            check = check + item
            
            if 'accesstoarray' in item or 'Malloc' in item:
                if not ('if' in check):
                    tmp = item.split(':')
                    num = int(tmp[len(tmp) - 2])
                    if num > 195:
                        count_i = count_i + 1
                        ilist.append(item)
            # if ('Malloc' in check) and ('MemMove' in item):
            #     l_secmem.append(item)

            if 'MemMove' in item and not ('Malloc' in check): # or 'snprintf' in item:
                if not (('if' in check) or (item in ilist)):
                    tmp = item.split(':')
                    num = int(tmp[len(tmp) - 2])
                    if num > 195:
                        count_i = count_i + 1
                        ilist.append(item)
    
    for key in params_shared:
        check = ''
        for item in params_shared[key]:
            
            if ('TEE_MemMove' in item) or ('TEE_CheckMemoryAccessRights' in item):
                continue

            if ('>' in item) or ('<' in item) or ('+' in item) or ('?' in item) or ('-' in item):
                continue

            if item in slist:
                continue
            
            count_s = count_s + 1
            slist.append(item)
    
    return count_i, ilist, count_s, slist, l_secmem


types = params_type()
funcs = switch_query()
alist = arrayaccess_query()
iflist = if_query()
p_i, s = dataflow_query(types, funcs, alist, iflist)
c_ue, l_ue = unencrypt_query()
c_i, l_i, c_s, l_s, l_sec = analysis(p_i, s)
print('Unencrypted Data Output: ' + str(c_ue))
print(l_ue)
print('Input Validation Weakness: ' + str(c_i))
print(l_i)
print('Shared Memory Overwrite: ' + str(c_s))
print(l_s)
