/** @defgroup OT Oblivious Transfer
*/

#include "emp-ot/ot/ot.h"
#include "emp-ot/ot/ideal.h"
#include "emp-ot/ot/co.h"
#include "emp-ot/ot/np.h"
#include "emp-ot/ot/shextension.h"
#include "emp-ot/ot/mextension_kos.h"
#include "emp-ot/ot/mextension_alsz.h"

template<typename IO>
using MOTExtension = MOTExtension_KOS<IO>;
//typedef MOTExtension_ALSZ MOTExtension;

#include "ot/iterated.h"


