/* Validation for transport-neutral anti-exfil protocol records. */

#ifndef KERN_ANTI_EXFIL_SEMANTIC_H
#define KERN_ANTI_EXFIL_SEMANTIC_H

#include "anti_exfil_types.h"

anti_exfil_result_t
anti_exfil_semantic_validate(const anti_exfil_message_t *message);

anti_exfil_result_t
anti_exfil_semantic_validate_transition(const anti_exfil_message_t *previous,
                                        const anti_exfil_message_t *next);

#endif // KERN_ANTI_EXFIL_SEMANTIC_H
