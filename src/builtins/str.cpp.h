/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * NOTE: this is NOT a header file. This is C++ file in the form
 * of a header file, just because it's faster to compile it this
 * way instead.
 */

#pragma once
#include "eval.h"
#include "evaltypes.cpp.h"
#include "syntax.h"

EvalValue builtin_int(EvalContext *ctx, ExprList *exprList)
{
    if (exprList->elems.size() != 1)
        throw InvalidNumberOfArgsEx(exprList->start, exprList->end);

    Construct *arg = exprList->elems[0].get();
    const EvalValue &val = RValue(arg->eval(ctx));

    if (!val.is<FlatSharedStr>())
        throw TypeErrorEx(arg->start, arg->end);

    try {

        return stol(string(val.get<FlatSharedStr>().get_view()));

    } catch (...) {

        throw TypeErrorEx(arg->start, arg->end);
    }
}

EvalValue builtin_split(EvalContext *ctx, ExprList *exprList)
{
    if (exprList->elems.size() != 2)
        throw InvalidNumberOfArgsEx(exprList->start, exprList->end);

    Construct *arg_str = exprList->elems[0].get();
    Construct *arg_delim = exprList->elems[1].get();

    const EvalValue &val_str = RValue(arg_str->eval(ctx));
    const EvalValue &val_delim = RValue(arg_delim->eval(ctx));

    if (!val_str.is<FlatSharedStr>())
        throw TypeErrorEx(arg_str->start, arg_str->end);

    if (!val_delim.is<FlatSharedStr>())
        throw TypeErrorEx(arg_delim->start, arg_delim->end);

    const FlatSharedStr &flat_str = val_str.get<FlatSharedStr>();
    const string_view &str = flat_str.get_view();
    const string_view &delim = val_delim.get<FlatSharedStr>().get_view();

    FlatSharedArray::vec_type vec;

    if (delim.size()) {

        size_t last = 0, next = 0;

        while ((next = str.find(delim, last)) != string::npos) {

            vec.emplace_back(
                EvalValue(FlatSharedStr(flat_str, last, next-last)),
                ctx->const_ctx
            );

            last = next + delim.size();
        }

        vec.emplace_back(
            EvalValue(FlatSharedStr(flat_str, last, str.size() - last)),
            ctx->const_ctx
        );

    } else {

        for (size_t i = 0; i < str.size(); i++) {
            vec.emplace_back(
                EvalValue(FlatSharedStr(flat_str, i, 1)),
                ctx->const_ctx
            );
        }
    }

    return EvalValue(FlatSharedArray(move(vec)));
}

EvalValue builtin_join(EvalContext *ctx, ExprList *exprList)
{
    if (exprList->elems.size() != 2)
        throw InvalidNumberOfArgsEx(exprList->start, exprList->end);

    Construct *arg_arr = exprList->elems[0].get();
    Construct *arg_delim = exprList->elems[1].get();

    const EvalValue &val_arr = RValue(arg_arr->eval(ctx));
    const EvalValue &val_delim = RValue(arg_delim->eval(ctx));

    if (!val_arr.is<FlatSharedArray>())
        throw TypeErrorEx(arg_arr->start, arg_arr->end);

    if (!val_delim.is<FlatSharedStr>())
        throw TypeErrorEx(arg_delim->start, arg_delim->end);

    const string_view &delim = val_delim.get<FlatSharedStr>().get_view();
    const FlatSharedArray &arr = val_arr.get<FlatSharedArray>();
    const FlatSharedArray::vec_type &vec = arr.get_ref();
    string result;

    for (size_t i = 0; i < arr.size(); i++) {

        const EvalValue &val = vec[arr.offset() + i].get();

        if (!val.is<FlatSharedStr>())
            throw TypeErrorEx(arg_arr->start, arg_arr->end);

        result += val.get<FlatSharedStr>().get_view();

        if (i != arr.size() - 1)
            result += delim;
    }

    return EvalValue(FlatSharedStr(move(result)));
}