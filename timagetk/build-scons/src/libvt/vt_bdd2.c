/*--- 

  Determine si un point est simple.
  On utilise la 26-connexite pour l'objet (points > 0)
  et la 6-connexite pour le fond (points == 0).

  On passe un voisinage du point considere sous forme 
  d'un tableau d'entiers. ATTENTION, LE POINT CENTRAL
  N'EST PAS DANS LE TABLEAU.

  Les indices du tableau correspondent donc au voisinage
  suivant :

	      0  1  2  -   9 10 11  -  17 18 19
	      3  4  5  -  12  . 13  -  20 21 22
	      6  7  8  -  14 15 16  -  23 24 25

  renvoie 1 si le point est simple (peut etre modifie
  sans changer la topologie).

---*/

#define RET_POS_VAL(a) return ((a)? !statInv : statInv)
#define RET_NEG_VAL(a) return ((a)? statInv : !statInv)
#define RETURN_SIMPLE(a) return (a)

/*-------------------- statistics --------------------*/
#if defined(_STATISTICS_)
extern int TESTS_nb;
#define VAL(tab,index) ( ++TESTS_nb ? tab[index] : tab[index] )
#else
#define VAL(tab,index) tab[index]
#endif
/*----------------------------------------------------*/

int IsSimple( register const int *V )
{
int statInv = 0;
if (VAL(V,12))
  if (VAL(V,10))
    if (VAL(V,4))
      if (VAL(V,13))
        if (VAL(V,15))
          RET_NEG_VAL(VAL(V,21));
        else
          Lbl0: if (VAL(V,21))
            RETURN_SIMPLE(!statInv);
          else
            RET_NEG_VAL(VAL(V,24));
      else
        if (VAL(V,15))
          Lbl1: if (VAL(V,21))
            RETURN_SIMPLE(!statInv);
          else
            RET_NEG_VAL(VAL(V,22));
        else
          Lbl2: if (VAL(V,21))
            RET_NEG_VAL(VAL(V,16));
          else
            Lbl3: if (VAL(V,16))
              if (VAL(V,22))
                RETURN_SIMPLE(statInv);
              else
                RET_NEG_VAL(VAL(V,24));
            else
              Lbl4: if (VAL(V,22))
                RET_NEG_VAL(VAL(V,24));
              else
                Lbl5: if (VAL(V,24))
                  RETURN_SIMPLE(!statInv);
                else
                  RET_NEG_VAL(VAL(V,25));
    else
      if (VAL(V,13))
        if (VAL(V,15))
          RET_POS_VAL(VAL(V,21));
        else
          if (VAL(V,7))
            RETURN_SIMPLE(statInv);
          else
            {goto Lbl0;}
      else
        if (VAL(V,5))
          if (VAL(V,15))
            RETURN_SIMPLE(statInv);
          else
            if (VAL(V,7))
              RETURN_SIMPLE(statInv);
            else
              {goto Lbl2;}
        else
          if (VAL(V,15))
            {goto Lbl1;}
          else
            if (VAL(V,7))
              {goto Lbl2;}
            else
              Lbl6: if (VAL(V,21))
                Lbl7: if (VAL(V,8))
                  RET_POS_VAL(VAL(V,16));
                else
                  RETURN_SIMPLE(!statInv);
              else
                Lbl8: if (VAL(V,8))
                  Lbl9: if (VAL(V,16))
                    Lbl10: if (VAL(V,22))
                      RET_NEG_VAL(VAL(V,24));
                    else
                      RET_POS_VAL(VAL(V,24));
                  else
                    RETURN_SIMPLE(statInv);
                else
                  if (VAL(V,16))
                    {goto Lbl10;}
                  else
                    {goto Lbl4;}
  else
    if (VAL(V,4))
      if (VAL(V,13))
        if (VAL(V,15))
          Lbl11: if (VAL(V,18))
            RET_POS_VAL(VAL(V,21));
          else
            RETURN_SIMPLE(!statInv);
        else
          Lbl12: if (VAL(V,18))
            RETURN_SIMPLE(statInv);
          else
            Lbl13: if (VAL(V,21))
              RETURN_SIMPLE(statInv);
            else
              RET_NEG_VAL(VAL(V,24));
      else
        if (VAL(V,15))
          Lbl14: if (VAL(V,11))
            if (VAL(V,18))
              RETURN_SIMPLE(statInv);
            else
              Lbl15: if (VAL(V,21))
                RETURN_SIMPLE(statInv);
              else
                RET_NEG_VAL(VAL(V,22));
          else
            Lbl16: if (VAL(V,18))
              {goto Lbl1;}
            else
              Lbl17: if (VAL(V,21))
                RETURN_SIMPLE(!statInv);
              else
                Lbl18: if (VAL(V,19))
                  RET_POS_VAL(VAL(V,22));
                else
                  RETURN_SIMPLE(!statInv);
        else
          Lbl19: if (VAL(V,11))
            if (VAL(V,18))
              RETURN_SIMPLE(statInv);
            else
              Lbl20: if (VAL(V,21))
                RETURN_SIMPLE(statInv);
              else
                {goto Lbl3;}
          else
            Lbl21: if (VAL(V,18))
              {goto Lbl2;}
            else
              Lbl22: if (VAL(V,21))
                RET_NEG_VAL(VAL(V,16));
              else
                Lbl23: if (VAL(V,16))
                  if (VAL(V,19))
                    Lbl24: if (VAL(V,22))
                      RET_NEG_VAL(VAL(V,24));
                    else
                      RETURN_SIMPLE(statInv);
                  else
                    RET_NEG_VAL(VAL(V,24));
                else
                  Lbl25: if (VAL(V,19))
                    Lbl26: if (VAL(V,22))
                      RET_POS_VAL(VAL(V,24));
                    else
                      RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,22))
                      RET_POS_VAL(VAL(V,24));
                    else
                      {goto Lbl5;}
    else
      if (VAL(V,1))
        if (VAL(V,13))
          if (VAL(V,15))
            RETURN_SIMPLE(statInv);
          else
            if (VAL(V,7))
              RETURN_SIMPLE(statInv);
            else
              {goto Lbl12;}
        else
          if (VAL(V,5))
            if (VAL(V,15))
              RETURN_SIMPLE(statInv);
            else
              if (VAL(V,7))
                RETURN_SIMPLE(statInv);
              else
                {goto Lbl19;}
          else
            if (VAL(V,15))
              {goto Lbl14;}
            else
              if (VAL(V,7))
                {goto Lbl19;}
              else
                if (VAL(V,11))
                  if (VAL(V,18))
                    RETURN_SIMPLE(statInv);
                  else
                    Lbl27: if (VAL(V,21))
                      RETURN_SIMPLE(statInv);
                    else
                      {goto Lbl8;}
                else
                  Lbl28: if (VAL(V,18))
                    {goto Lbl6;}
                  else
                    Lbl29: if (VAL(V,21))
                      {goto Lbl7;}
                    else
                      Lbl30: if (VAL(V,8))
                        Lbl31: if (VAL(V,16))
                          Lbl32: if (VAL(V,19))
                            {goto Lbl26;}
                          else
                            RET_POS_VAL(VAL(V,24));
                        else
                          RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,16))
                          {goto Lbl32;}
                        else
                          {goto Lbl25;}
      else
        if (VAL(V,13))
          if (VAL(V,15))
            {goto Lbl11;}
          else
            if (VAL(V,7))
              {goto Lbl12;}
            else
              if (VAL(V,18))
                {goto Lbl0;}
              else
                {statInv=(!statInv);goto Lbl13;}
        else
          if (VAL(V,5))
            if (VAL(V,15))
              {goto Lbl14;}
            else
              if (VAL(V,7))
                {goto Lbl19;}
              else
                if (VAL(V,11))
                  if (VAL(V,18))
                    {goto Lbl2;}
                  else
                    Lbl33: if (VAL(V,21))
                      RET_NEG_VAL(VAL(V,16));
                    else
                      Lbl34: if (VAL(V,16))
                        if (VAL(V,22))
                          RETURN_SIMPLE(statInv);
                        else
                          RET_POS_VAL(VAL(V,24));
                      else
                        {goto Lbl26;}
                else
                  if (VAL(V,18))
                    Lbl35: if (VAL(V,21))
                      RET_POS_VAL(VAL(V,16));
                    else
                      {goto Lbl9;}
                  else
                    Lbl36: if (VAL(V,21))
                      RET_POS_VAL(VAL(V,16));
                    else
                      {goto Lbl31;}
          else
            if (VAL(V,15))
              if (VAL(V,11))
                if (VAL(V,18))
                  {goto Lbl1;}
                else
                  {statInv=(!statInv);goto Lbl15;}
              else
                if (VAL(V,2))
                  RETURN_SIMPLE(statInv);
                else
                  {goto Lbl16;}
            else
              if (VAL(V,7))
                if (VAL(V,11))
                  if (VAL(V,18))
                    {goto Lbl2;}
                  else
                    Lbl37: if (VAL(V,21))
                      RET_NEG_VAL(VAL(V,16));
                    else
                      Lbl38: if (VAL(V,16))
                        {goto Lbl24;}
                      else
                        {goto Lbl26;}
                else
                  if (VAL(V,2))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl21;}
              else
                if (VAL(V,11))
                  if (VAL(V,18))
                    {goto Lbl6;}
                  else
                    Lbl39: if (VAL(V,21))
                      {goto Lbl7;}
                    else
                      Lbl40: if (VAL(V,8))
                        Lbl41: if (VAL(V,16))
                          {goto Lbl26;}
                        else
                          RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl26;}
                else
                  if (VAL(V,2))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl28;}
else
  if (VAL(V,10))
    if (VAL(V,4))
      if (VAL(V,13))
        if (VAL(V,15))
          Lbl42: if (VAL(V,20))
            RET_POS_VAL(VAL(V,21));
          else
            RETURN_SIMPLE(!statInv);
        else
          Lbl43: if (VAL(V,14))
            Lbl44: if (VAL(V,20))
              RETURN_SIMPLE(statInv);
            else
              {goto Lbl13;}
          else
            Lbl45: if (VAL(V,20))
              {goto Lbl0;}
            else
              Lbl46: if (VAL(V,21))
                RETURN_SIMPLE(!statInv);
              else
                Lbl47: if (VAL(V,23))
                  RET_POS_VAL(VAL(V,24));
                else
                  RETURN_SIMPLE(!statInv);
      else
        if (VAL(V,15))
          Lbl48: if (VAL(V,20))
            RETURN_SIMPLE(statInv);
          else
            {goto Lbl15;}
        else
          Lbl49: if (VAL(V,14))
            Lbl50: if (VAL(V,20))
              RETURN_SIMPLE(statInv);
            else
              {goto Lbl20;}
          else
            Lbl51: if (VAL(V,20))
              {goto Lbl2;}
            else
              Lbl52: if (VAL(V,21))
                RET_NEG_VAL(VAL(V,16));
              else
                Lbl53: if (VAL(V,16))
                  if (VAL(V,22))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl47;}
                else
                  Lbl54: if (VAL(V,22))
                    {goto Lbl47;}
                  else
                    Lbl55: if (VAL(V,23))
                      RETURN_SIMPLE(statInv);
                    else
                      Lbl56: if (VAL(V,24))
                        RETURN_SIMPLE(statInv);
                      else
                        RET_NEG_VAL(VAL(V,25));
    else
      if (VAL(V,3))
        if (VAL(V,13))
          if (VAL(V,15))
            RETURN_SIMPLE(statInv);
          else
            if (VAL(V,7))
              RETURN_SIMPLE(statInv);
            else
              {goto Lbl43;}
        else
          if (VAL(V,5))
            if (VAL(V,15))
              RETURN_SIMPLE(statInv);
            else
              if (VAL(V,7))
                RETURN_SIMPLE(statInv);
              else
                {goto Lbl49;}
          else
            if (VAL(V,15))
              {goto Lbl48;}
            else
              if (VAL(V,7))
                {goto Lbl49;}
              else
                Lbl57: if (VAL(V,14))
                  Lbl58: if (VAL(V,20))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl27;}
                else
                  Lbl59: if (VAL(V,20))
                    {goto Lbl6;}
                  else
                    Lbl60: if (VAL(V,21))
                      {goto Lbl7;}
                    else
                      Lbl61: if (VAL(V,8))
                        Lbl62: if (VAL(V,16))
                          Lbl63: if (VAL(V,22))
                            {goto Lbl47;}
                          else
                            RETURN_SIMPLE(statInv);
                        else
                          RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,16))
                          {goto Lbl63;}
                        else
                          {goto Lbl54;}
      else
        if (VAL(V,13))
          if (VAL(V,15))
            {goto Lbl42;}
          else
            if (VAL(V,7))
              {goto Lbl43;}
            else
              Lbl64: if (VAL(V,14))
                Lbl65: if (VAL(V,20))
                  {goto Lbl0;}
                else
                  {statInv=(!statInv);goto Lbl13;}
              else
                if (VAL(V,6))
                  RETURN_SIMPLE(statInv);
                else
                  {goto Lbl45;}
        else
          if (VAL(V,5))
            if (VAL(V,15))
              {goto Lbl48;}
            else
              if (VAL(V,7))
                {goto Lbl49;}
              else
                Lbl66: if (VAL(V,14))
                  Lbl67: if (VAL(V,20))
                    {goto Lbl2;}
                  else
                    {goto Lbl33;}
                else
                  if (VAL(V,6))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl51;}
          else
            if (VAL(V,15))
              Lbl68: if (VAL(V,20))
                {goto Lbl1;}
              else
                {statInv=(!statInv);goto Lbl15;}
            else
              if (VAL(V,7))
                Lbl69: if (VAL(V,14))
                  if (VAL(V,20))
                    {goto Lbl2;}
                  else
                    {goto Lbl37;}
                else
                  Lbl70: if (VAL(V,20))
                    {goto Lbl35;}
                  else
                    Lbl71: if (VAL(V,21))
                      RET_POS_VAL(VAL(V,16));
                    else
                      {goto Lbl62;}
              else
                Lbl72: if (VAL(V,14))
                  Lbl73: if (VAL(V,20))
                    {goto Lbl6;}
                  else
                    {goto Lbl39;}
                else
                  if (VAL(V,6))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl59;}
  else
    if (VAL(V,4))
      if (VAL(V,9))
        if (VAL(V,13))
          if (VAL(V,15))
            Lbl74: if (VAL(V,18))
              RETURN_SIMPLE(statInv);
            else
              Lbl75: if (VAL(V,20))
                RETURN_SIMPLE(statInv);
              else
                RET_NEG_VAL(VAL(V,21));
          else
            Lbl76: if (VAL(V,18))
              RETURN_SIMPLE(statInv);
            else
              if (VAL(V,14))
                {goto Lbl44;}
              else
                Lbl77: if (VAL(V,20))
                  {goto Lbl13;}
                else
                  if (VAL(V,21))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl47;}
        else
          if (VAL(V,15))
            Lbl78: if (VAL(V,11))
              if (VAL(V,18))
                RETURN_SIMPLE(statInv);
              else
                {goto Lbl48;}
            else
              Lbl79: if (VAL(V,18))
                {goto Lbl48;}
              else
                if (VAL(V,20))
                  RETURN_SIMPLE(statInv);
                else
                  if (VAL(V,21))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl18;}
          else
            Lbl80: if (VAL(V,11))
              if (VAL(V,18))
                RETURN_SIMPLE(statInv);
              else
                if (VAL(V,14))
                  {goto Lbl50;}
                else
                  Lbl81: if (VAL(V,20))
                    {goto Lbl20;}
                  else
                    if (VAL(V,21))
                      RETURN_SIMPLE(statInv);
                    else
                      {goto Lbl53;}
            else
              Lbl82: if (VAL(V,18))
                {goto Lbl49;}
              else
                if (VAL(V,14))
                  if (VAL(V,20))
                    RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,21))
                      RETURN_SIMPLE(statInv);
                    else
                      {goto Lbl23;}
                else
                  Lbl83: if (VAL(V,20))
                    {goto Lbl22;}
                  else
                    if (VAL(V,21))
                      RET_NEG_VAL(VAL(V,16));
                    else
                      Lbl84: if (VAL(V,16))
                        Lbl85: if (VAL(V,19))
                          {goto Lbl63;}
                        else
                          {goto Lbl47;}
                      else
                        Lbl86: if (VAL(V,19))
                          RETURN_SIMPLE(statInv);
                        else
                          if (VAL(V,22))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl55;}
      else
        if (VAL(V,13))
          if (VAL(V,15))
            Lbl87: if (VAL(V,18))
              {goto Lbl42;}
            else
              if (VAL(V,17))
                {statInv=(!statInv);goto Lbl75;}
              else
                RETURN_SIMPLE(!statInv);
          else
            Lbl88: if (VAL(V,18))
              {goto Lbl43;}
            else
              if (VAL(V,14))
                if (VAL(V,17))
                  Lbl89: if (VAL(V,20))
                    {goto Lbl13;}
                  else
                    RETURN_SIMPLE(statInv);
                else
                  {goto Lbl13;}
              else
                Lbl90: if (VAL(V,17))
                  Lbl91: if (VAL(V,20))
                    {statInv=(!statInv);goto Lbl13;}
                  else
                    RET_POS_VAL(VAL(V,21));
                else
                  if (VAL(V,20))
                    {statInv=(!statInv);goto Lbl13;}
                  else
                    {goto Lbl46;}
        else
          if (VAL(V,15))
            Lbl92: if (VAL(V,11))
              if (VAL(V,18))
                {goto Lbl48;}
              else
                if (VAL(V,17))
                  Lbl93: if (VAL(V,20))
                    {goto Lbl15;}
                  else
                    RETURN_SIMPLE(statInv);
                else
                  {goto Lbl15;}
            else
              Lbl94: if (VAL(V,18))
                {goto Lbl68;}
              else
                if (VAL(V,17))
                  Lbl95: if (VAL(V,20))
                    {goto Lbl17;}
                  else
                    RET_POS_VAL(VAL(V,21));
                else
                  {goto Lbl17;}
          else
            Lbl96: if (VAL(V,11))
              if (VAL(V,18))
                {goto Lbl49;}
              else
                if (VAL(V,14))
                  if (VAL(V,17))
                    Lbl97: if (VAL(V,20))
                      {goto Lbl20;}
                    else
                      RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl20;}
                else
                  Lbl98: if (VAL(V,17))
                    Lbl99: if (VAL(V,20))
                      {goto Lbl33;}
                    else
                      Lbl100: if (VAL(V,21))
                        RET_NEG_VAL(VAL(V,16));
                      else
                        RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,20))
                      {goto Lbl33;}
                    else
                      {goto Lbl52;}
            else
              Lbl101: if (VAL(V,18))
                {goto Lbl69;}
              else
                if (VAL(V,14))
                  if (VAL(V,17))
                    Lbl102: if (VAL(V,20))
                      {goto Lbl22;}
                    else
                      {goto Lbl100;}
                  else
                    {goto Lbl22;}
                else
                  Lbl103: if (VAL(V,17))
                    Lbl104: if (VAL(V,20))
                      {goto Lbl36;}
                    else
                      Lbl105: if (VAL(V,21))
                        RET_POS_VAL(VAL(V,16));
                      else
                        RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,20))
                      {goto Lbl36;}
                    else
                      if (VAL(V,21))
                        RET_POS_VAL(VAL(V,16));
                      else
                        {goto Lbl84;}
    else
      if (VAL(V,3))
        if (VAL(V,1))
          if (VAL(V,9))
            if (VAL(V,13))
              if (VAL(V,15))
                RETURN_SIMPLE(statInv);
              else
                if (VAL(V,7))
                  RETURN_SIMPLE(statInv);
                else
                  {goto Lbl76;}
            else
              if (VAL(V,5))
                if (VAL(V,15))
                  RETURN_SIMPLE(statInv);
                else
                  if (VAL(V,7))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl80;}
              else
                if (VAL(V,15))
                  {goto Lbl78;}
                else
                  if (VAL(V,7))
                    {goto Lbl80;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          {goto Lbl58;}
                        else
                          Lbl106: if (VAL(V,20))
                            {goto Lbl27;}
                          else
                            if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl61;}
                    else
                      Lbl107: if (VAL(V,18))
                        {goto Lbl57;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            RETURN_SIMPLE(statInv);
                          else
                            if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl30;}
                        else
                          Lbl108: if (VAL(V,20))
                            {goto Lbl29;}
                          else
                            if (VAL(V,21))
                              {goto Lbl7;}
                            else
                              Lbl109: if (VAL(V,8))
                                RETURN_SIMPLE(statInv);
                              else
                                if (VAL(V,16))
                                  RETURN_SIMPLE(statInv);
                                else
                                  {goto Lbl86;}
          else
            if (VAL(V,13))
              if (VAL(V,15))
                RETURN_SIMPLE(statInv);
              else
                if (VAL(V,7))
                  RETURN_SIMPLE(statInv);
                else
                  {goto Lbl88;}
            else
              if (VAL(V,5))
                if (VAL(V,15))
                  RETURN_SIMPLE(statInv);
                else
                  if (VAL(V,7))
                    RETURN_SIMPLE(statInv);
                  else
                    {goto Lbl96;}
              else
                if (VAL(V,15))
                  {goto Lbl92;}
                else
                  if (VAL(V,7))
                    {goto Lbl96;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl57;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,17))
                            if (VAL(V,20))
                              {goto Lbl27;}
                            else
                              RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl27;}
                        else
                          Lbl110: if (VAL(V,17))
                            Lbl111: if (VAL(V,20))
                              {goto Lbl39;}
                            else
                              Lbl112: if (VAL(V,21))
                                {goto Lbl7;}
                              else
                                RETURN_SIMPLE(statInv);
                          else
                            if (VAL(V,20))
                              {goto Lbl39;}
                            else
                              {goto Lbl60;}
                    else
                      Lbl113: if (VAL(V,18))
                        Lbl114: if (VAL(V,14))
                          {goto Lbl73;}
                        else
                          RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          Lbl115: if (VAL(V,17))
                            Lbl116: if (VAL(V,20))
                              {goto Lbl29;}
                            else
                              {goto Lbl112;}
                          else
                            {goto Lbl29;}
                        else
                          Lbl117: if (VAL(V,17))
                            RETURN_SIMPLE(statInv);
                          else
                            if (VAL(V,20))
                              RETURN_SIMPLE(statInv);
                            else
                              if (VAL(V,21))
                                RETURN_SIMPLE(statInv);
                              else
                                {goto Lbl109;}
        else
          if (VAL(V,9))
            if (VAL(V,13))
              if (VAL(V,15))
                {goto Lbl74;}
              else
                if (VAL(V,7))
                  {goto Lbl76;}
                else
                  if (VAL(V,18))
                    {goto Lbl43;}
                  else
                    if (VAL(V,14))
                      if (VAL(V,20))
                        RETURN_SIMPLE(statInv);
                      else
                        {statInv=(!statInv);goto Lbl0;}
                    else
                      {goto Lbl91;}
            else
              if (VAL(V,5))
                if (VAL(V,15))
                  {goto Lbl78;}
                else
                  if (VAL(V,7))
                    {goto Lbl80;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl49;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            RETURN_SIMPLE(statInv);
                          else
                            Lbl118: if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl34;}
                        else
                          {goto Lbl99;}
                    else
                      if (VAL(V,18))
                        if (VAL(V,14))
                          if (VAL(V,20))
                            RETURN_SIMPLE(statInv);
                          else
                            Lbl119: if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl9;}
                        else
                          {goto Lbl70;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            RETURN_SIMPLE(statInv);
                          else
                            if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl31;}
                        else
                          {goto Lbl104;}
              else
                if (VAL(V,15))
                  if (VAL(V,11))
                    if (VAL(V,18))
                      {goto Lbl48;}
                    else
                      if (VAL(V,20))
                        RETURN_SIMPLE(statInv);
                      else
                        {statInv=(!statInv);goto Lbl1;}
                  else
                    if (VAL(V,2))
                      RETURN_SIMPLE(statInv);
                    else
                      {goto Lbl79;}
                else
                  if (VAL(V,7))
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl49;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            RETURN_SIMPLE(statInv);
                          else
                            Lbl120: if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl38;}
                        else
                          if (VAL(V,20))
                            {goto Lbl37;}
                          else
                            if (VAL(V,21))
                              RET_NEG_VAL(VAL(V,16));
                            else
                              {goto Lbl62;}
                    else
                      if (VAL(V,2))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl82;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl57;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            RETURN_SIMPLE(statInv);
                          else
                            Lbl121: if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl40;}
                        else
                          {goto Lbl111;}
                    else
                      if (VAL(V,2))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl107;}
          else
            if (VAL(V,13))
              if (VAL(V,15))
                {goto Lbl87;}
              else
                if (VAL(V,7))
                  {goto Lbl88;}
                else
                  if (VAL(V,18))
                    if (VAL(V,14))
                      {goto Lbl65;}
                    else
                      RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,14))
                      Lbl122: if (VAL(V,17))
                        {goto Lbl91;}
                      else
                        {statInv=(!statInv);goto Lbl13;}
                    else
                      RETURN_SIMPLE(statInv);
            else
              if (VAL(V,5))
                if (VAL(V,15))
                  {goto Lbl92;}
                else
                  if (VAL(V,7))
                    {goto Lbl96;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        if (VAL(V,14))
                          {goto Lbl67;}
                        else
                          RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          Lbl123: if (VAL(V,17))
                            {goto Lbl99;}
                          else
                            {goto Lbl33;}
                        else
                          RETURN_SIMPLE(statInv);
                    else
                      if (VAL(V,18))
                        if (VAL(V,14))
                          Lbl124: if (VAL(V,20))
                            {goto Lbl35;}
                          else
                            Lbl125: if (VAL(V,21))
                              RET_POS_VAL(VAL(V,16));
                            else
                              {goto Lbl41;}
                        else
                          RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          Lbl126: if (VAL(V,17))
                            {goto Lbl104;}
                          else
                            {goto Lbl36;}
                        else
                          RETURN_SIMPLE(statInv);
              else
                if (VAL(V,15))
                  Lbl127: if (VAL(V,11))
                    Lbl128: if (VAL(V,18))
                      {goto Lbl68;}
                    else
                      if (VAL(V,17))
                        Lbl129: if (VAL(V,20))
                          {statInv=(!statInv);goto Lbl15;}
                        else
                          RET_POS_VAL(VAL(V,21));
                      else
                        {statInv=(!statInv);goto Lbl15;}
                  else
                    if (VAL(V,2))
                      RETURN_SIMPLE(statInv);
                    else
                      {goto Lbl94;}
                else
                  if (VAL(V,7))
                    Lbl130: if (VAL(V,11))
                      Lbl131: if (VAL(V,18))
                        {goto Lbl69;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,17))
                            Lbl132: if (VAL(V,20))
                              {goto Lbl37;}
                            else
                              {goto Lbl100;}
                          else
                            {goto Lbl37;}
                        else
                          if (VAL(V,17))
                            Lbl133: if (VAL(V,20))
                              {goto Lbl125;}
                            else
                              {goto Lbl105;}
                          else
                            if (VAL(V,20))
                              {goto Lbl125;}
                            else
                              {goto Lbl71;}
                    else
                      if (VAL(V,2))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl101;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl114;}
                      else
                        if (VAL(V,14))
                          Lbl134: if (VAL(V,17))
                            {goto Lbl111;}
                          else
                            {goto Lbl39;}
                        else
                          RETURN_SIMPLE(statInv);
                    else
                      if (VAL(V,2))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl113;}
      else
        if (VAL(V,1))
          if (VAL(V,9))
            if (VAL(V,13))
              if (VAL(V,15))
                {goto Lbl74;}
              else
                if (VAL(V,7))
                  {goto Lbl76;}
                else
                  if (VAL(V,18))
                    RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,14))
                      if (VAL(V,20))
                        {goto Lbl13;}
                      else
                        {statInv=(!statInv);goto Lbl0;}
                    else
                      if (VAL(V,6))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl77;}
            else
              if (VAL(V,5))
                if (VAL(V,15))
                  {goto Lbl78;}
                else
                  if (VAL(V,7))
                    {goto Lbl80;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            {goto Lbl20;}
                          else
                            {goto Lbl118;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl81;}
                    else
                      if (VAL(V,18))
                        {goto Lbl66;}
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            {goto Lbl22;}
                          else
                            if (VAL(V,21))
                              RET_NEG_VAL(VAL(V,16));
                            else
                              {goto Lbl31;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl83;}
              else
                if (VAL(V,15))
                  if (VAL(V,11))
                    if (VAL(V,18))
                      RETURN_SIMPLE(statInv);
                    else
                      if (VAL(V,20))
                        {goto Lbl15;}
                      else
                        {statInv=(!statInv);goto Lbl1;}
                  else
                    Lbl135: if (VAL(V,18))
                      {goto Lbl68;}
                    else
                      {goto Lbl95;}
                else
                  if (VAL(V,7))
                    if (VAL(V,11))
                      if (VAL(V,18))
                        RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            {goto Lbl20;}
                          else
                            {goto Lbl120;}
                        else
                          if (VAL(V,20))
                            {goto Lbl119;}
                          else
                            if (VAL(V,21))
                              RETURN_SIMPLE(statInv);
                            else
                              {goto Lbl62;}
                    else
                      Lbl136: if (VAL(V,18))
                        {goto Lbl69;}
                      else
                        if (VAL(V,14))
                          {goto Lbl102;}
                        else
                          {goto Lbl104;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          if (VAL(V,20))
                            {goto Lbl27;}
                          else
                            {goto Lbl121;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl106;}
                    else
                      Lbl137: if (VAL(V,18))
                        {goto Lbl72;}
                      else
                        if (VAL(V,14))
                          {goto Lbl116;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl108;}
          else
            if (VAL(V,13))
              Lbl138: if (VAL(V,15))
                {goto Lbl87;}
              else
                if (VAL(V,7))
                  {goto Lbl88;}
                else
                  if (VAL(V,18))
                    {goto Lbl64;}
                  else
                    if (VAL(V,14))
                      {goto Lbl122;}
                    else
                      if (VAL(V,6))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl90;}
            else
              if (VAL(V,5))
                Lbl139: if (VAL(V,15))
                  {goto Lbl92;}
                else
                  if (VAL(V,7))
                    {goto Lbl96;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl66;}
                      else
                        if (VAL(V,14))
                          {goto Lbl123;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl98;}
                    else
                      if (VAL(V,18))
                        Lbl140: if (VAL(V,14))
                          {goto Lbl124;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl70;}
                      else
                        if (VAL(V,14))
                          {goto Lbl126;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl103;}
              else
                if (VAL(V,15))
                  if (VAL(V,11))
                    {goto Lbl128;}
                  else
                    RETURN_SIMPLE(statInv);
                else
                  if (VAL(V,7))
                    if (VAL(V,11))
                      {goto Lbl131;}
                    else
                      RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,11))
                      Lbl141: if (VAL(V,18))
                        {goto Lbl72;}
                      else
                        if (VAL(V,14))
                          {goto Lbl134;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl110;}
                    else
                      Lbl142: if (VAL(V,18))
                        RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,14))
                          RETURN_SIMPLE(statInv);
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl117;}
        else
          if (VAL(V,9))
            if (VAL(V,13))
              if (VAL(V,15))
                if (VAL(V,18))
                  {goto Lbl42;}
                else
                  {statInv=(!statInv);goto Lbl75;}
              else
                if (VAL(V,7))
                  if (VAL(V,18))
                    {goto Lbl43;}
                  else
                    if (VAL(V,14))
                      {goto Lbl89;}
                    else
                      {goto Lbl91;}
                else
                  if (VAL(V,18))
                    {goto Lbl64;}
                  else
                    if (VAL(V,14))
                      {goto Lbl91;}
                    else
                      if (VAL(V,6))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl91;}
            else
              if (VAL(V,5))
                if (VAL(V,15))
                  if (VAL(V,11))
                    if (VAL(V,18))
                      {goto Lbl48;}
                    else
                      {goto Lbl93;}
                  else
                    {goto Lbl135;}
                else
                  if (VAL(V,7))
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl49;}
                      else
                        if (VAL(V,14))
                          {goto Lbl97;}
                        else
                          {goto Lbl99;}
                    else
                      {goto Lbl136;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl66;}
                      else
                        if (VAL(V,14))
                          {goto Lbl99;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl99;}
                    else
                      if (VAL(V,18))
                        {goto Lbl140;}
                      else
                        if (VAL(V,14))
                          {goto Lbl104;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl104;}
              else
                if (VAL(V,15))
                  if (VAL(V,11))
                    if (VAL(V,18))
                      {goto Lbl68;}
                    else
                      {goto Lbl129;}
                  else
                    if (VAL(V,2))
                      RETURN_SIMPLE(statInv);
                    else
                      {goto Lbl135;}
                else
                  if (VAL(V,7))
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl69;}
                      else
                        if (VAL(V,14))
                          {goto Lbl132;}
                        else
                          {goto Lbl133;}
                    else
                      if (VAL(V,2))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl136;}
                  else
                    if (VAL(V,11))
                      if (VAL(V,18))
                        {goto Lbl72;}
                      else
                        if (VAL(V,14))
                          {goto Lbl111;}
                        else
                          if (VAL(V,6))
                            RETURN_SIMPLE(statInv);
                          else
                            {goto Lbl111;}
                    else
                      if (VAL(V,2))
                        RETURN_SIMPLE(statInv);
                      else
                        {goto Lbl137;}
          else
            if (VAL(V,0))
              if (VAL(V,13))
                RETURN_SIMPLE(statInv);
              else
                if (VAL(V,5))
                  RETURN_SIMPLE(statInv);
                else
                  if (VAL(V,15))
                    RETURN_SIMPLE(statInv);
                  else
                    if (VAL(V,7))
                      RETURN_SIMPLE(statInv);
                    else
                      if (VAL(V,11))
                        RETURN_SIMPLE(statInv);
                      else
                        if (VAL(V,2))
                          RETURN_SIMPLE(statInv);
                        else
                          {goto Lbl142;}
            else
              if (VAL(V,13))
                {goto Lbl138;}
              else
                if (VAL(V,5))
                  {goto Lbl139;}
                else
                  if (VAL(V,15))
                    {goto Lbl127;}
                  else
                    if (VAL(V,7))
                      {goto Lbl130;}
                    else
                      if (VAL(V,11))
                        {goto Lbl141;}
                      else
                        if (VAL(V,2))
                          {goto Lbl142;}
                        else
                          if (VAL(V,18))
                            {goto Lbl72;}
                          else
                            if (VAL(V,14))
                              {goto Lbl115;}
                            else
                              if (VAL(V,6))
                                {goto Lbl117;}
                              else
                                if (VAL(V,17))
                                  {goto Lbl108;}
                                else
                                  if (VAL(V,20))
                                    {goto Lbl29;}
                                  else
                                    if (VAL(V,21))
                                      {goto Lbl7;}
                                    else
                                      if (VAL(V,8))
                                        {goto Lbl84;}
                                      else
                                        if (VAL(V,16))
                                          {goto Lbl85;}
                                        else
                                          if (VAL(V,19))
                                            {goto Lbl54;}
                                          else
                                            if (VAL(V,22))
                                              {goto Lbl47;}
                                            else
                                              if (VAL(V,23))
                                                {goto Lbl5;}
                                              else
                                                {statInv=(!statInv);goto Lbl56;}

}
