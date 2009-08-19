// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Boolean_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "NativeFunction.h"
#include "GnashException.h"
#include "VM.h" 
#include "Object.h" // for getObjectInterface

namespace gnash {

namespace {    
    as_value boolean_tostring(const fn_call& fn);
    as_value boolean_valueof(const fn_call& fn);
    as_value boolean_ctor(const fn_call& fn);
    void attachBooleanInterface(as_object& o);
    as_object* getBooleanInterface();
}

class Boolean_as: public Proxy
{

public:

    explicit Boolean_as(bool val)
        :
        _val(val)
    {
    }
    
    bool value() const { return _val; }

private:

    bool _val;
    
};

// extern (used by Global.cpp)
void
boolean_class_init(as_object& where, const ObjectURI& uri)
{
    // This is going to be the global Boolean "class"/"function"
    as_object* proto = getBooleanInterface();
    Global_as* gl = getGlobal(where);
    as_object* cl = gl->createClass(&boolean_ctor, proto);

    // Register _global.Boolean
    where.init_member(getName(uri), cl, as_object::DefaultFlags,
            getNamespace(uri));

}

void
registerBooleanNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(boolean_valueof, 107, 0);
    vm.registerNative(boolean_tostring, 107, 1);
    vm.registerNative(boolean_ctor, 107, 2);
}

namespace {


void
attachBooleanInterface(as_object& o)
{
    VM& vm = getVM(o);
    o.init_member("valueOf", vm.getNative(107, 0));
    o.init_member("toString", vm.getNative(107, 1));
}

as_object*
getBooleanInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o )
    {
        o = new as_object(getObjectInterface());
        VM::get().addStatic(o.get());

        attachBooleanInterface(*o);
    }
    return o.get();
}


as_value
boolean_tostring(const fn_call& fn)
{
    Boolean_as* obj = checkType<Boolean_as>(fn.this_ptr.get());
    if (obj->value()) return as_value("true");
    return as_value("false");
}


as_value
boolean_valueof(const fn_call& fn) 
{
    Boolean_as* obj = checkType<Boolean_as>(fn.this_ptr.get());
    return as_value(obj->value());
}

as_value
boolean_ctor(const fn_call& fn)
{

    if (!fn.isInstantiation()) {
        if (!fn.nargs) return as_value();
        return as_value(fn.arg(0).to_bool());
    }

    const bool val = fn.nargs ? fn.arg(0).to_bool() : false;

    as_object* obj = fn.this_ptr.get();
    obj->setProxy(new Boolean_as(val));
    return as_value();

}

} // anonymous namespace
} // gnash namespace

