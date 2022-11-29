#include "VCDValue.h"

/*!
*/
VCDValue::VCDValue    (
    VCDBit  val
){
    type = VCD_SCALAR;
    value.val_bit = val;
}

/*!
*/
VCDValue::VCDValue    (
    VCDBitVector *  val
){
    type = VCD_VECTOR;
    value.val_vector= val;
}

/*!
*/
VCDValue::VCDValue    (
    VCDReal val
){
    type = VCD_REAL;
    value.val_real = val;
}


VCDValueType   VCDValue::get_type(){
    return type;
}


/*!
*/
VCDBit       VCDValue::get_value_bit(){
    return value.val_bit;
}


/*!
*/
VCDBitVector * VCDValue::get_value_vector(){
    return value.val_vector;
}


/*!
*/
VCDReal      VCDValue::get_value_real(){
    return value.val_real;
}

