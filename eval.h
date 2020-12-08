/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include "evalvalue.h"
#include <map>

using namespace std;

class LValue {

    EvalValue val;

    void type_checks() const {
        assert(val.get_type()->t != Type::t_lval);
        assert(val.get_type()->t != Type::t_undefid);
    }

public:

    const bool is_const;

    LValue(const EvalValue &val, bool is_const = false)
        : val(val)
        , is_const(is_const)
    {
        type_checks();
    }

    LValue(EvalValue &&val, bool is_const = false)
        : val(forward<EvalValue>(val))
        , is_const(is_const)
    {
        type_checks();
    }

    void put(const EvalValue &v) { val = v; type_checks(); }
    void put(EvalValue &&v) { val = forward<EvalValue>(v); type_checks(); }

    EvalValue eval() const { return val; }
};

class EvalContext {

public:

    typedef map<string, LValue, less<>> SymbolsType;
    EvalContext *const parent;
    const bool const_ctx;
    const bool func_ctx;
    SymbolsType symbols;

    EvalContext(const EvalContext &rhs) = delete;
    EvalContext(EvalContext &&rhs) = delete;

    EvalContext(EvalContext *parent = nullptr,
                bool const_ctx = false,
                bool func_ctx = false);

    static const SymbolsType builtins;
    static const SymbolsType const_builtins;
};
