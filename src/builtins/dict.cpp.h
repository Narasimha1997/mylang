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

EvalValue
builtin_find_dict(const FlatSharedDictObj &obj, const EvalValue &key)
{
    const DictObject &dictObj = obj.get();
    const DictObject::inner_type &data = dictObj.get_ref();

    const auto &it = data.find(key);

    if (it == data.end())
        return EvalValue();

    return it->second.get();
}

EvalValue
builtin_erase_dict(LValue *lval, const EvalValue &key)
{
    DictObject &dictObj = lval->getval<FlatSharedDictObj>().get();
    DictObject::inner_type &data = dictObj.get_ref();
    return data.erase(key) > 0;
}

EvalValue
builtin_insert_dict(LValue *lval, const EvalValue &key, const EvalValue &val)
{
    DictObject &dictObj = lval->getval<FlatSharedDictObj>().get();
    DictObject::inner_type &data = dictObj.get_ref();
    const auto &it = data.insert(make_pair(key, LValue(val, false)));
    return it.second;
}

static EvalValue
dict_keys(const DictObject::inner_type &data)
{
    FlatSharedArray::vec_type result;

    for (auto const &e : data) {
        result.emplace_back(e.first, false);
    }

    return FlatSharedArray(move(result));
}

static EvalValue
dict_values(const DictObject::inner_type &data)
{
    FlatSharedArray::vec_type result;

    for (auto const &e : data) {
        result.emplace_back(e.second.get(), false);
    }

    return FlatSharedArray(move(result));
}

static EvalValue
dict_kvpairs(const DictObject::inner_type &data)
{
    FlatSharedArray::vec_type result;

    for (auto const &e : data) {

        FlatSharedArray::vec_type pair_arr;
        pair_arr.emplace_back(e.first, false);
        pair_arr.emplace_back(e.second.get(), false);
        result.emplace_back(FlatSharedArray(move(pair_arr)), false);
    }

    return FlatSharedArray(move(result));
}

static EvalValue
dict_1arg_func(EvalContext *ctx,
               ExprList *exprList,
               EvalValue (*f)(const DictObject::inner_type &))
{
    if (exprList->elems.size() != 1)
        throw InvalidNumberOfArgsEx(exprList->start, exprList->end);

    Construct *arg0 = exprList->elems[0].get();
    const EvalValue &val0 = RValue(arg0->eval(ctx));

    if (!val0.is<FlatSharedDictObj>())
        throw TypeErrorEx("Expected dict object", arg0->start, arg0->end);

    const DictObject::inner_type &data
        = val0.get<FlatSharedDictObj>()->get_ref();

    return f(data);
}

EvalValue
builtin_keys(EvalContext *ctx, ExprList *exprList)
{
    return dict_1arg_func(ctx, exprList, &dict_keys);
}

EvalValue
builtin_values(EvalContext *ctx, ExprList *exprList)
{
    return dict_1arg_func(ctx, exprList, &dict_values);
}

EvalValue
builtin_kvpairs(EvalContext *ctx, ExprList *exprList)
{
    return dict_1arg_func(ctx, exprList, &dict_kvpairs);
}
