/** @defgroup OT Oblivious Transfer
*/

#include "emp-ot/ot.h"
#include "emp-ot/ideal.h"
#include "emp-ot/co.h"
#include "emp-ot/np.h"
#include "emp-ot/shextension.h"
#include "emp-ot/mextension_kos.h"
#include "emp-ot/mextension_alsz.h"

template<typename IO>
using MOTExtension = MOTExtension_KOS<IO>;
//typedef MOTExtension_ALSZ MOTExtension;

#include "emp-ot/iterated.h"


