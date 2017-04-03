
#include <vt_boolean.h>

#define BDDV_RET_POS_VAL(a) return ((a)? !statInv : statInv)
#define BDDV_RET_NEG_VAL(a) return ((a)? statInv : !statInv)
#define BDDV_RET_SIMPLE(a) return (a)

/*-------------------- statistics --------------------*/
#if defined(_STATISTICS_)
extern int TESTS_nb;
#define VAL(tab,index) ( ++TESTS_nb ? tab[index] : tab[index] )
#else
#define VAL(tab,index) tab[index]
#endif
/*----------------------------------------------------*/

int IsSimple3D_6NEP( register const int *V )
{int statInv = 0;
if (VAL(V,12))
  if (VAL(V,13))
    if (VAL(V,10))
      if (VAL(V,9))
        if (VAL(V,15))
          if (VAL(V,16))
            if (VAL(V,4))
              if (VAL(V,3))
                if (VAL(V,1))
                  if (VAL(V,0))
                    if (VAL(V,5))
                      if (VAL(V,7))
                        if (VAL(V,8))
                          if (VAL(V,2))
                            if (VAL(V,6))
                              if (VAL(V,11))
                                if (VAL(V,14))
                                  if (VAL(V,20))
                                    if (VAL(V,17))
                                      if (VAL(V,18))
                                        if (VAL(V,19))
                                          if (VAL(V,22))
                                            Lbl0: if (VAL(V,21))
                                              BDDV_RET_NEG_VAL(VAL(V,24));
                                            else
                                              BDDV_RET_SIMPLE(!statInv);
                                          else
                                            Lbl1: if (VAL(V,23))
                                              BDDV_RET_SIMPLE(!statInv);
                                            else
                                              {goto Lbl0;}
                                        else
                                          Lbl2: if (VAL(V,22))
                                            BDDV_RET_NEG_VAL(VAL(V,21));
                                          else
                                            {goto Lbl1;}
                                      else
                                        Lbl3: if (VAL(V,22))
                                          Lbl4: if (VAL(V,23))
                                            Lbl5: if (VAL(V,21))
                                              Lbl6: if (VAL(V,24))
                                                BDDV_RET_POS_VAL(VAL(V,25));
                                              else
                                                BDDV_RET_SIMPLE(statInv);
                                            else
                                              BDDV_RET_SIMPLE(!statInv);
                                          else
                                            BDDV_RET_NEG_VAL(VAL(V,21));
                                        else
                                          {goto Lbl1;}
                                    else
                                      if (VAL(V,18))
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                      else
                                        {goto Lbl3;}
                                  else
                                    if (VAL(V,18))
                                      Lbl7: if (VAL(V,19))
                                        if (VAL(V,22))
                                          Lbl8: if (VAL(V,21))
                                            Lbl9: if (VAL(V,24))
                                              BDDV_RET_POS_VAL(VAL(V,25));
                                            else
                                              BDDV_RET_SIMPLE(!statInv);
                                          else
                                            BDDV_RET_SIMPLE(!statInv);
                                        else
                                          {goto Lbl0;}
                                      else
                                        if (VAL(V,22))
                                          BDDV_RET_NEG_VAL(VAL(V,21));
                                        else
                                          {goto Lbl0;}
                                    else
                                      if (VAL(V,22))
                                        {goto Lbl8;}
                                      else
                                        Lbl10: if (VAL(V,21))
                                          BDDV_RET_POS_VAL(VAL(V,24));
                                        else
                                          BDDV_RET_SIMPLE(!statInv);
                                else
                                  Lbl11: if (VAL(V,20))
                                    if (VAL(V,17))
                                      if (VAL(V,18))
                                        {goto Lbl7;}
                                      else
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      BDDV_RET_NEG_VAL(VAL(V,21));
                                  else
                                    Lbl12: if (VAL(V,18))
                                      if (VAL(V,19))
                                        if (VAL(V,22))
                                          {goto Lbl8;}
                                        else
                                          BDDV_RET_NEG_VAL(VAL(V,21));
                                      else
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      Lbl13: if (VAL(V,22))
                                        {goto Lbl5;}
                                      else
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                              else
                                Lbl14: if (VAL(V,14))
                                  if (VAL(V,20))
                                    if (VAL(V,17))
                                      if (VAL(V,18))
                                        {goto Lbl3;}
                                      else
                                        Lbl15: if (VAL(V,22))
                                          {goto Lbl4;}
                                        else
                                          Lbl16: if (VAL(V,23))
                                            {goto Lbl10;}
                                          else
                                            BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      if (VAL(V,18))
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                      else
                                        {goto Lbl15;}
                                  else
                                    if (VAL(V,18))
                                      BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      {goto Lbl13;}
                                else
                                  BDDV_RET_SIMPLE(statInv);
                            else
                              if (VAL(V,11))
                                if (VAL(V,14))
                                  BDDV_RET_SIMPLE(statInv);
                                else
                                  {goto Lbl11;}
                              else
                                BDDV_RET_SIMPLE(statInv);
                          else
                            if (VAL(V,6))
                              if (VAL(V,11))
                                BDDV_RET_SIMPLE(statInv);
                              else
                                {goto Lbl14;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        Lbl17: if (VAL(V,2))
                          if (VAL(V,11))
                            if (VAL(V,14))
                              Lbl18: if (VAL(V,20))
                                if (VAL(V,17))
                                  Lbl19: if (VAL(V,18))
                                    Lbl20: if (VAL(V,19))
                                      Lbl21: if (VAL(V,22))
                                        Lbl22: if (VAL(V,23))
                                          Lbl23: if (VAL(V,21))
                                            {goto Lbl6;}
                                          else
                                            BDDV_RET_SIMPLE(statInv);
                                        else
                                          BDDV_RET_SIMPLE(statInv);
                                      else
                                        BDDV_RET_SIMPLE(statInv);
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              if (VAL(V,20))
                                if (VAL(V,17))
                                  if (VAL(V,18))
                                    if (VAL(V,19))
                                      if (VAL(V,22))
                                        if (VAL(V,21))
                                          {goto Lbl9;}
                                        else
                                          BDDV_RET_SIMPLE(statInv);
                                      else
                                        BDDV_RET_SIMPLE(statInv);
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                Lbl24: if (VAL(V,18))
                                  if (VAL(V,19))
                                    Lbl25: if (VAL(V,22))
                                      {goto Lbl23;}
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                    else
                      if (VAL(V,7))
                        Lbl26: if (VAL(V,6))
                          if (VAL(V,11))
                            Lbl27: if (VAL(V,14))
                              {goto Lbl18;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            if (VAL(V,14))
                              if (VAL(V,20))
                                if (VAL(V,17))
                                  if (VAL(V,18))
                                    if (VAL(V,22))
                                      {goto Lbl22;}
                                    else
                                      Lbl28: if (VAL(V,23))
                                        {statInv=(!statInv);goto Lbl0;}
                                      else
                                        BDDV_RET_SIMPLE(statInv);
                                  else
                                    {goto Lbl21;}
                                else
                                  Lbl29: if (VAL(V,18))
                                    BDDV_RET_SIMPLE(statInv);
                                  else
                                    {goto Lbl21;}
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        Lbl30: if (VAL(V,11))
                          Lbl31: if (VAL(V,14))
                            {goto Lbl18;}
                          else
                            if (VAL(V,20))
                              if (VAL(V,17))
                                {goto Lbl24;}
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          Lbl32: if (VAL(V,14))
                            if (VAL(V,20))
                              if (VAL(V,17))
                                Lbl33: if (VAL(V,18))
                                  {goto Lbl21;}
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        {goto Lbl26;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    if (VAL(V,7))
                      if (VAL(V,6))
                        if (VAL(V,11))
                          {goto Lbl27;}
                        else
                          {goto Lbl32;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      Lbl34: if (VAL(V,11))
                        {goto Lbl27;}
                      else
                        BDDV_RET_SIMPLE(statInv);
              else
                if (VAL(V,1))
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        {goto Lbl17;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      if (VAL(V,2))
                        if (VAL(V,11))
                          {goto Lbl31;}
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                  else
                    if (VAL(V,7))
                      BDDV_RET_SIMPLE(statInv);
                    else
                      {goto Lbl34;}
                else
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        {goto Lbl30;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      {goto Lbl34;}
                  else
                    if (VAL(V,7))
                      {goto Lbl34;}
                    else
                      BDDV_RET_SIMPLE(statInv);
            else
              {goto Lbl30;}
          else
            if (VAL(V,4))
              if (VAL(V,3))
                if (VAL(V,1))
                  if (VAL(V,0))
                    if (VAL(V,5))
                      if (VAL(V,7))
                        if (VAL(V,2))
                          if (VAL(V,6))
                            if (VAL(V,11))
                              if (VAL(V,14))
                                if (VAL(V,20))
                                  if (VAL(V,17))
                                    if (VAL(V,18))
                                      if (VAL(V,19))
                                        {goto Lbl1;}
                                      else
                                        {goto Lbl2;}
                                    else
                                      Lbl35: if (VAL(V,22))
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                      else
                                        {goto Lbl16;}
                                  else
                                    if (VAL(V,18))
                                      BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      {goto Lbl35;}
                                else
                                  if (VAL(V,18))
                                    if (VAL(V,19))
                                      if (VAL(V,22))
                                        {goto Lbl0;}
                                      else
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      BDDV_RET_NEG_VAL(VAL(V,21));
                                  else
                                    BDDV_RET_NEG_VAL(VAL(V,21));
                              else
                                Lbl36: if (VAL(V,20))
                                  if (VAL(V,17))
                                    if (VAL(V,18))
                                      if (VAL(V,19))
                                        if (VAL(V,22))
                                          {statInv=(!statInv);goto Lbl10;}
                                        else
                                          BDDV_RET_SIMPLE(statInv);
                                      else
                                        BDDV_RET_SIMPLE(statInv);
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                            else
                              Lbl37: if (VAL(V,14))
                                if (VAL(V,20))
                                  if (VAL(V,17))
                                    if (VAL(V,18))
                                      Lbl38: if (VAL(V,22))
                                        BDDV_RET_SIMPLE(statInv);
                                      else
                                        {goto Lbl28;}
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                          else
                            if (VAL(V,11))
                              if (VAL(V,14))
                                BDDV_RET_SIMPLE(statInv);
                              else
                                {goto Lbl36;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          if (VAL(V,6))
                            if (VAL(V,11))
                              BDDV_RET_SIMPLE(statInv);
                            else
                              {goto Lbl37;}
                          else
                            BDDV_RET_SIMPLE(statInv);
                      else
                        if (VAL(V,2))
                          if (VAL(V,11))
                            if (VAL(V,14))
                              Lbl39: if (VAL(V,20))
                                if (VAL(V,17))
                                  if (VAL(V,18))
                                    if (VAL(V,19))
                                      if (VAL(V,22))
                                        if (VAL(V,23))
                                          BDDV_RET_POS_VAL(VAL(V,21));
                                        else
                                          {statInv=(!statInv);goto Lbl10;}
                                      else
                                        {goto Lbl28;}
                                    else
                                      {goto Lbl38;}
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              if (VAL(V,20))
                                if (VAL(V,17))
                                  if (VAL(V,18))
                                    if (VAL(V,19))
                                      if (VAL(V,22))
                                        {statInv=(!statInv);goto Lbl0;}
                                      else
                                        BDDV_RET_SIMPLE(statInv);
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                    else
                      if (VAL(V,7))
                        if (VAL(V,6))
                          if (VAL(V,11))
                            if (VAL(V,14))
                              {goto Lbl39;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            if (VAL(V,14))
                              if (VAL(V,20))
                                if (VAL(V,17))
                                  if (VAL(V,18))
                                    Lbl40: if (VAL(V,22))
                                      {goto Lbl28;}
                                    else
                                      BDDV_RET_SIMPLE(statInv);
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        Lbl41: if (VAL(V,11))
                          if (VAL(V,14))
                            if (VAL(V,20))
                              if (VAL(V,17))
                                if (VAL(V,18))
                                  if (VAL(V,19))
                                    {goto Lbl40;}
                                  else
                                    BDDV_RET_SIMPLE(statInv);
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  if (VAL(V,5))
                    BDDV_RET_SIMPLE(statInv);
                  else
                    if (VAL(V,7))
                      if (VAL(V,6))
                        {goto Lbl41;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
              else
                if (VAL(V,1))
                  if (VAL(V,5))
                    if (VAL(V,7))
                      BDDV_RET_SIMPLE(statInv);
                    else
                      if (VAL(V,2))
                        {goto Lbl41;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
            else
              {goto Lbl41;}
        else
          if (VAL(V,4))
            if (VAL(V,3))
              if (VAL(V,1))
                if (VAL(V,0))
                  if (VAL(V,5))
                    if (VAL(V,2))
                      if (VAL(V,11))
                        if (VAL(V,20))
                          if (VAL(V,17))
                            if (VAL(V,18))
                              if (VAL(V,19))
                                BDDV_RET_SIMPLE(!statInv);
                              else
                                Lbl42: if (VAL(V,22))
                                  BDDV_RET_NEG_VAL(VAL(V,21));
                                else
                                  BDDV_RET_SIMPLE(!statInv);
                            else
                              BDDV_RET_NEG_VAL(VAL(V,21));
                          else
                            BDDV_RET_NEG_VAL(VAL(V,21));
                        else
                          Lbl43: if (VAL(V,18))
                            if (VAL(V,19))
                              if (VAL(V,22))
                                BDDV_RET_SIMPLE(!statInv);
                              else
                                BDDV_RET_NEG_VAL(VAL(V,21));
                            else
                              BDDV_RET_NEG_VAL(VAL(V,21));
                          else
                            BDDV_RET_NEG_VAL(VAL(V,21));
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    Lbl44: if (VAL(V,11))
                      if (VAL(V,20))
                        if (VAL(V,17))
                          Lbl45: if (VAL(V,18))
                            if (VAL(V,19))
                              {statInv=(!statInv);goto Lbl42;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              if (VAL(V,1))
                if (VAL(V,5))
                  if (VAL(V,2))
                    {goto Lbl44;}
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
          else
            {goto Lbl44;}
      else
        if (VAL(V,15))
          if (VAL(V,16))
            if (VAL(V,4))
              if (VAL(V,3))
                if (VAL(V,1))
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        if (VAL(V,2))
                          if (VAL(V,6))
                            if (VAL(V,11))
                              if (VAL(V,14))
                                if (VAL(V,20))
                                  if (VAL(V,18))
                                    if (VAL(V,19))
                                      if (VAL(V,22))
                                        {goto Lbl4;}
                                      else
                                        BDDV_RET_NEG_VAL(VAL(V,21));
                                    else
                                      BDDV_RET_NEG_VAL(VAL(V,21));
                                  else
                                    {goto Lbl15;}
                                else
                                  {goto Lbl12;}
                              else
                                Lbl46: if (VAL(V,20))
                                  BDDV_RET_SIMPLE(statInv);
                                else
                                  {goto Lbl24;}
                            else
                              Lbl47: if (VAL(V,14))
                                if (VAL(V,20))
                                  {goto Lbl29;}
                                else
                                  BDDV_RET_SIMPLE(statInv);
                              else
                                BDDV_RET_SIMPLE(statInv);
                          else
                            if (VAL(V,11))
                              if (VAL(V,14))
                                BDDV_RET_SIMPLE(statInv);
                              else
                                {goto Lbl46;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          if (VAL(V,6))
                            if (VAL(V,11))
                              BDDV_RET_SIMPLE(statInv);
                            else
                              {goto Lbl47;}
                          else
                            BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        if (VAL(V,6))
                          if (VAL(V,11))
                            if (VAL(V,14))
                              Lbl48: if (VAL(V,20))
                                if (VAL(V,18))
                                  {goto Lbl20;}
                                else
                                  {goto Lbl21;}
                              else
                                {goto Lbl24;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            if (VAL(V,14))
                              if (VAL(V,20))
                                {goto Lbl33;}
                              else
                                BDDV_RET_SIMPLE(statInv);
                            else
                              BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    if (VAL(V,7))
                      if (VAL(V,6))
                        Lbl49: if (VAL(V,11))
                          if (VAL(V,14))
                            if (VAL(V,20))
                              {goto Lbl19;}
                            else
                              BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
              else
                if (VAL(V,1))
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        if (VAL(V,2))
                          if (VAL(V,11))
                            if (VAL(V,14))
                              {goto Lbl48;}
                            else
                              if (VAL(V,20))
                                {goto Lbl24;}
                              else
                                BDDV_RET_SIMPLE(statInv);
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      if (VAL(V,2))
                        {goto Lbl49;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  if (VAL(V,5))
                    if (VAL(V,7))
                      if (VAL(V,8))
                        {goto Lbl49;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
            else
              {goto Lbl49;}
          else
            BDDV_RET_SIMPLE(statInv);
        else
          BDDV_RET_SIMPLE(statInv);
    else
      if (VAL(V,15))
        if (VAL(V,16))
          if (VAL(V,4))
            if (VAL(V,3))
              if (VAL(V,5))
                if (VAL(V,7))
                  if (VAL(V,8))
                    if (VAL(V,6))
                      if (VAL(V,14))
                        if (VAL(V,20))
                          {goto Lbl15;}
                        else
                          {goto Lbl13;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                if (VAL(V,7))
                  if (VAL(V,6))
                    Lbl50: if (VAL(V,14))
                      if (VAL(V,20))
                        {goto Lbl21;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
            else
              if (VAL(V,5))
                if (VAL(V,7))
                  if (VAL(V,8))
                    {goto Lbl50;}
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
          else
            {goto Lbl50;}
        else
          BDDV_RET_SIMPLE(statInv);
      else
        BDDV_RET_SIMPLE(statInv);
  else
    if (VAL(V,10))
      if (VAL(V,9))
        if (VAL(V,15))
          if (VAL(V,4))
            if (VAL(V,3))
              if (VAL(V,1))
                if (VAL(V,0))
                  if (VAL(V,7))
                    if (VAL(V,6))
                      if (VAL(V,14))
                        if (VAL(V,20))
                          if (VAL(V,17))
                            if (VAL(V,18))
                              {goto Lbl1;}
                            else
                              {goto Lbl16;}
                          else
                            if (VAL(V,18))
                              BDDV_RET_NEG_VAL(VAL(V,21));
                            else
                              {goto Lbl16;}
                        else
                          BDDV_RET_NEG_VAL(VAL(V,21));
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    Lbl51: if (VAL(V,14))
                      if (VAL(V,20))
                        if (VAL(V,17))
                          if (VAL(V,18))
                            {goto Lbl28;}
                          else
                            BDDV_RET_SIMPLE(statInv);
                        else
                          BDDV_RET_SIMPLE(statInv);
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                if (VAL(V,7))
                  if (VAL(V,6))
                    {goto Lbl51;}
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
            else
              BDDV_RET_SIMPLE(statInv);
          else
            {goto Lbl51;}
        else
          if (VAL(V,4))
            if (VAL(V,3))
              if (VAL(V,1))
                if (VAL(V,0))
                  if (VAL(V,20))
                    if (VAL(V,17))
                      if (VAL(V,18))
                        BDDV_RET_SIMPLE(!statInv);
                      else
                        BDDV_RET_NEG_VAL(VAL(V,21));
                    else
                      BDDV_RET_NEG_VAL(VAL(V,21));
                  else
                    BDDV_RET_NEG_VAL(VAL(V,21));
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              BDDV_RET_SIMPLE(statInv);
          else
            if (VAL(V,20))
              if (VAL(V,17))
                if (VAL(V,18))
                  BDDV_RET_POS_VAL(VAL(V,21));
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              BDDV_RET_SIMPLE(statInv);
      else
        BDDV_RET_SIMPLE(statInv);
    else
      if (VAL(V,15))
        if (VAL(V,4))
          if (VAL(V,3))
            if (VAL(V,7))
              if (VAL(V,6))
                if (VAL(V,14))
                  if (VAL(V,20))
                    {goto Lbl16;}
                  else
                    BDDV_RET_NEG_VAL(VAL(V,21));
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              BDDV_RET_SIMPLE(statInv);
          else
            BDDV_RET_SIMPLE(statInv);
        else
          if (VAL(V,14))
            if (VAL(V,20))
              {goto Lbl28;}
            else
              BDDV_RET_SIMPLE(statInv);
          else
            BDDV_RET_SIMPLE(statInv);
      else
        BDDV_RET_SIMPLE(statInv);
else
  if (VAL(V,13))
    if (VAL(V,10))
      if (VAL(V,15))
        if (VAL(V,16))
          if (VAL(V,4))
            if (VAL(V,1))
              if (VAL(V,5))
                if (VAL(V,7))
                  if (VAL(V,8))
                    if (VAL(V,2))
                      if (VAL(V,11))
                        {goto Lbl12;}
                      else
                        BDDV_RET_SIMPLE(statInv);
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  if (VAL(V,2))
                    Lbl52: if (VAL(V,11))
                      {goto Lbl24;}
                    else
                      BDDV_RET_SIMPLE(statInv);
                  else
                    BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              if (VAL(V,5))
                if (VAL(V,7))
                  if (VAL(V,8))
                    {goto Lbl52;}
                  else
                    BDDV_RET_SIMPLE(statInv);
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
          else
            {goto Lbl52;}
        else
          BDDV_RET_SIMPLE(statInv);
      else
        if (VAL(V,4))
          if (VAL(V,1))
            if (VAL(V,5))
              if (VAL(V,2))
                if (VAL(V,11))
                  {goto Lbl43;}
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              BDDV_RET_SIMPLE(statInv);
          else
            BDDV_RET_SIMPLE(statInv);
        else
          if (VAL(V,11))
            {goto Lbl45;}
          else
            BDDV_RET_SIMPLE(statInv);
    else
      if (VAL(V,15))
        if (VAL(V,16))
          if (VAL(V,4))
            if (VAL(V,5))
              if (VAL(V,7))
                if (VAL(V,8))
                  {goto Lbl13;}
                else
                  BDDV_RET_SIMPLE(statInv);
              else
                BDDV_RET_SIMPLE(statInv);
            else
              BDDV_RET_SIMPLE(statInv);
          else
            {goto Lbl25;}
        else
          BDDV_RET_SIMPLE(statInv);
      else
        BDDV_RET_SIMPLE(statInv);
  else
    BDDV_RET_SIMPLE(statInv);

}
