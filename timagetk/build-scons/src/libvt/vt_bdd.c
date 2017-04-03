
#include <vt_bdd.h>

static int statInv;

/*------- Definition des fonctions statiques ----------*/


static int statProc( int *V )
{
statInv = 0;
if (V[4])
  if (V[10])
    if (V[13])
      if (V[12])
        if (V[15])
          return !V[21];
        else
          Lbl0: if (V[21])
            return 1;
          else
            return !V[24];
      else
        Lbl1: if (V[15])
          Lbl2: if (V[20])
            return V[21];
          else
            return 1;
        else
          Lbl3: if (V[14])
            Lbl4: if (V[20])
              return 0;
            else
              Lbl5: if (V[21])
                return 0;
              else
                return !V[24];
          else
            Lbl6: if (V[20])
              {goto Lbl0;}
            else
              Lbl7: if (V[21])
                return 1;
              else
                Lbl8: if (V[23])
                  return V[24];
                else
                  return 1;
    else
      if (V[12])
        Lbl9: if (V[15])
          Lbl10: if (V[21])
            return 1;
          else
            return !V[22];
        else
          Lbl11: if (V[21])
            return !V[16];
          else
            Lbl12: if (V[16])
              if (V[22])
                return 0;
              else
                return !V[24];
            else
              Lbl13: if (V[22])
                return !V[24];
              else
                Lbl14: if (V[24])
                  return 1;
                else
                  return !V[25];
      else
        Lbl15: if (V[15])
          Lbl16: if (V[20])
            return 0;
          else
            Lbl17: if (V[21])
              return 0;
            else
              return !V[22];
        else
          Lbl18: if (V[14])
            Lbl19: if (V[20])
              return 0;
            else
              Lbl20: if (V[21])
                return 0;
              else
                {goto Lbl12;}
          else
            Lbl21: if (V[20])
              {goto Lbl11;}
            else
              Lbl22: if (V[21])
                return !V[16];
              else
                Lbl23: if (V[16])
                  if (V[22])
                    return 0;
                  else
                    {goto Lbl8;}
                else
                  Lbl24: if (V[22])
                    {goto Lbl8;}
                  else
                    Lbl25: if (V[23])
                      return 0;
                    else
                      Lbl26: if (V[24])
                        return 0;
                      else
                        return !V[25];
  else
    if (V[13])
      if (V[12])
        Lbl27: if (V[18])
          Lbl28: if (V[15])
            return V[21];
          else
            return 0;
        else
          if (V[15])
            return 1;
          else
            {goto Lbl5;}
      else
        Lbl29: if (V[9])
          if (V[18])
            return 0;
          else
            if (V[15])
              Lbl30: if (V[20])
                return 0;
              else
                return !V[21];
            else
              Lbl31: if (V[14])
                {goto Lbl4;}
              else
                Lbl32: if (V[20])
                  {goto Lbl5;}
                else
                  if (V[21])
                    return 0;
                  else
                    {goto Lbl8;}
        else
          Lbl33: if (V[18])
            {goto Lbl1;}
          else
            if (V[15])
              Lbl34: if (V[17])
                {statInv=(!statInv);goto Lbl30;}
              else
                return 1;
            else
              Lbl35: if (V[14])
                if (V[17])
                  Lbl36: if (V[20])
                    {goto Lbl5;}
                  else
                    return 0;
                else
                  {goto Lbl5;}
              else
                Lbl37: if (V[17])
                  Lbl38: if (V[20])
                    {statInv=(!statInv);goto Lbl5;}
                  else
                    return V[21];
                else
                  if (V[20])
                    {statInv=(!statInv);goto Lbl5;}
                  else
                    {goto Lbl7;}
    else
      if (V[11])
        if (V[12])
          Lbl39: if (V[18])
            return 0;
          else
            if (V[15])
              {goto Lbl17;}
            else
              {goto Lbl20;}
        else
          Lbl40: if (V[9])
            if (V[18])
              return 0;
            else
              if (V[15])
                {goto Lbl16;}
              else
                Lbl41: if (V[14])
                  {goto Lbl19;}
                else
                  Lbl42: if (V[20])
                    {goto Lbl20;}
                  else
                    if (V[21])
                      return 0;
                    else
                      {goto Lbl23;}
          else
            Lbl43: if (V[18])
              {goto Lbl15;}
            else
              if (V[15])
                Lbl44: if (V[17])
                  Lbl45: if (V[20])
                    {goto Lbl17;}
                  else
                    return 0;
                else
                  {goto Lbl17;}
              else
                Lbl46: if (V[14])
                  if (V[17])
                    Lbl47: if (V[20])
                      {goto Lbl20;}
                    else
                      return 0;
                  else
                    {goto Lbl20;}
                else
                  Lbl48: if (V[17])
                    Lbl49: if (V[20])
                      Lbl50: if (V[21])
                        return !V[16];
                      else
                        Lbl51: if (V[16])
                          if (V[22])
                            return 0;
                          else
                            return V[24];
                        else
                          Lbl52: if (V[22])
                            return V[24];
                          else
                            return 0;
                    else
                      Lbl53: if (V[21])
                        return !V[16];
                      else
                        return 0;
                  else
                    if (V[20])
                      {goto Lbl50;}
                    else
                      {goto Lbl22;}
      else
        if (V[12])
          Lbl54: if (V[18])
            {goto Lbl9;}
          else
            if (V[15])
              Lbl55: if (V[21])
                return 1;
              else
                Lbl56: if (V[19])
                  return V[22];
                else
                  return 1;
            else
              Lbl57: if (V[21])
                return !V[16];
              else
                Lbl58: if (V[19])
                  Lbl59: if (V[16])
                    if (V[22])
                      return !V[24];
                    else
                      return 0;
                  else
                    {goto Lbl52;}
                else
                  if (V[16])
                    return !V[24];
                  else
                    Lbl60: if (V[22])
                      return V[24];
                    else
                      {goto Lbl14;}
        else
          Lbl61: if (V[9])
            if (V[18])
              {goto Lbl15;}
            else
              if (V[15])
                Lbl62: if (V[20])
                  return 0;
                else
                  if (V[21])
                    return 0;
                  else
                    {goto Lbl56;}
              else
                Lbl63: if (V[14])
                  if (V[20])
                    return 0;
                  else
                    if (V[21])
                      return 0;
                    else
                      {goto Lbl58;}
                else
                  Lbl64: if (V[20])
                    {goto Lbl57;}
                  else
                    if (V[21])
                      return !V[16];
                    else
                      Lbl65: if (V[19])
                        Lbl66: if (V[16])
                          Lbl67: if (V[22])
                            {goto Lbl8;}
                          else
                            return 0;
                        else
                          return 0;
                      else
                        if (V[16])
                          {goto Lbl8;}
                        else
                          Lbl68: if (V[22])
                            return 0;
                          else
                            {goto Lbl25;}
          else
            Lbl69: if (V[18])
              Lbl70: if (V[15])
                Lbl71: if (V[20])
                  {goto Lbl10;}
                else
                  {statInv=(!statInv);goto Lbl17;}
              else
                Lbl72: if (V[14])
                  if (V[20])
                    {goto Lbl11;}
                  else
                    Lbl73: if (V[21])
                      return !V[16];
                    else
                      {goto Lbl59;}
                else
                  Lbl74: if (V[20])
                    Lbl75: if (V[21])
                      return V[16];
                    else
                      Lbl76: if (V[16])
                        Lbl77: if (V[22])
                          return !V[24];
                        else
                          return V[24];
                      else
                        return 0;
                  else
                    Lbl78: if (V[21])
                      return V[16];
                    else
                      {goto Lbl66;}
            else
              if (V[15])
                Lbl79: if (V[17])
                  Lbl80: if (V[20])
                    {goto Lbl55;}
                  else
                    return V[21];
                else
                  {goto Lbl55;}
              else
                Lbl81: if (V[14])
                  if (V[17])
                    Lbl82: if (V[20])
                      {goto Lbl57;}
                    else
                      {goto Lbl53;}
                  else
                    {goto Lbl57;}
                else
                  Lbl83: if (V[17])
                    Lbl84: if (V[20])
                      Lbl85: if (V[21])
                        return V[16];
                      else
                        Lbl86: if (V[19])
                          Lbl87: if (V[16])
                            {goto Lbl52;}
                          else
                            return 0;
                        else
                          if (V[16])
                            return V[24];
                          else
                            return 0;
                    else
                      Lbl88: if (V[21])
                        return V[16];
                      else
                        return 0;
                  else
                    if (V[20])
                      {goto Lbl85;}
                    else
                      if (V[21])
                        return V[16];
                      else
                        {goto Lbl65;}
else
  if (V[10])
    if (V[13])
      if (V[7])
        if (V[12])
          {goto Lbl28;}
        else
          if (V[3])
            return 0;
          else
            {goto Lbl1;}
      else
        if (V[12])
          Lbl89: if (V[15])
            return V[21];
          else
            {goto Lbl0;}
        else
          if (V[3])
            Lbl90: if (V[15])
              return 0;
            else
              {goto Lbl3;}
          else
            Lbl91: if (V[15])
              {goto Lbl2;}
            else
              if (V[14])
                Lbl92: if (V[20])
                  {goto Lbl0;}
                else
                  {statInv=(!statInv);goto Lbl5;}
              else
                if (V[6])
                  return 0;
                else
                  {goto Lbl6;}
    else
      if (V[5])
        if (V[7])
          if (V[12])
            return 0;
          else
            if (V[3])
              return 0;
            else
              {goto Lbl15;}
        else
          if (V[12])
            Lbl93: if (V[15])
              return 0;
            else
              {goto Lbl11;}
          else
            if (V[3])
              Lbl94: if (V[15])
                return 0;
              else
                {goto Lbl18;}
            else
              Lbl95: if (V[15])
                {goto Lbl16;}
              else
                if (V[14])
                  Lbl96: if (V[20])
                    {goto Lbl11;}
                  else
                    {goto Lbl50;}
                else
                  if (V[6])
                    return 0;
                  else
                    {goto Lbl21;}
      else
        if (V[7])
          if (V[12])
            {goto Lbl9;}
          else
            if (V[3])
              {goto Lbl15;}
            else
              {goto Lbl70;}
        else
          if (V[12])
            Lbl97: if (V[15])
              {goto Lbl10;}
            else
              Lbl98: if (V[21])
                Lbl99: if (V[8])
                  return V[16];
                else
                  return 1;
              else
                Lbl100: if (V[8])
                  {goto Lbl76;}
                else
                  if (V[16])
                    {goto Lbl77;}
                  else
                    {goto Lbl13;}
          else
            if (V[3])
              Lbl101: if (V[15])
                {goto Lbl16;}
              else
                if (V[14])
                  Lbl102: if (V[20])
                    return 0;
                  else
                    Lbl103: if (V[21])
                      return 0;
                    else
                      {goto Lbl100;}
                else
                  Lbl104: if (V[20])
                    {goto Lbl98;}
                  else
                    Lbl105: if (V[21])
                      {goto Lbl99;}
                    else
                      Lbl106: if (V[8])
                        {goto Lbl66;}
                      else
                        Lbl107: if (V[16])
                          {goto Lbl67;}
                        else
                          {goto Lbl24;}
            else
              Lbl108: if (V[15])
                {goto Lbl71;}
              else
                if (V[14])
                  Lbl109: if (V[20])
                    {goto Lbl98;}
                  else
                    Lbl110: if (V[21])
                      {goto Lbl99;}
                    else
                      Lbl111: if (V[8])
                        {goto Lbl87;}
                      else
                        {goto Lbl52;}
                else
                  if (V[6])
                    return 0;
                  else
                    {goto Lbl104;}
  else
    if (V[13])
      if (V[1])
        if (V[7])
          if (V[12])
            return 0;
          else
            if (V[3])
              return 0;
            else
              {goto Lbl29;}
        else
          if (V[12])
            if (V[18])
              return 0;
            else
              Lbl112: if (V[15])
                return 0;
              else
                {goto Lbl5;}
          else
            if (V[3])
              if (V[9])
                if (V[18])
                  return 0;
                else
                  if (V[15])
                    return 0;
                  else
                    {goto Lbl31;}
              else
                if (V[18])
                  {goto Lbl90;}
                else
                  if (V[15])
                    return 0;
                  else
                    {goto Lbl35;}
            else
              if (V[9])
                if (V[18])
                  return 0;
                else
                  if (V[15])
                    {goto Lbl30;}
                  else
                    if (V[14])
                      if (V[20])
                        {goto Lbl5;}
                      else
                        {statInv=(!statInv);goto Lbl0;}
                    else
                      if (V[6])
                        return 0;
                      else
                        {goto Lbl32;}
              else
                Lbl113: if (V[18])
                  {goto Lbl91;}
                else
                  if (V[15])
                    {goto Lbl34;}
                  else
                    if (V[14])
                      Lbl114: if (V[17])
                        {goto Lbl38;}
                      else
                        {statInv=(!statInv);goto Lbl5;}
                    else
                      if (V[6])
                        return 0;
                      else
                        {goto Lbl37;}
      else
        if (V[7])
          if (V[12])
            {goto Lbl27;}
          else
            if (V[3])
              {goto Lbl29;}
            else
              if (V[9])
                if (V[18])
                  {goto Lbl1;}
                else
                  if (V[15])
                    {statInv=(!statInv);goto Lbl30;}
                  else
                    if (V[14])
                      {goto Lbl36;}
                    else
                      {goto Lbl38;}
              else
                if (V[0])
                  return 0;
                else
                  {goto Lbl33;}
        else
          if (V[12])
            if (V[18])
              {goto Lbl89;}
            else
              {statInv=(!statInv);goto Lbl112;}
          else
            if (V[3])
              if (V[9])
                if (V[18])
                  {goto Lbl90;}
                else
                  if (V[15])
                    {goto Lbl30;}
                  else
                    if (V[14])
                      if (V[20])
                        return 0;
                      else
                        {statInv=(!statInv);goto Lbl0;}
                    else
                      {goto Lbl38;}
              else
                if (V[18])
                  if (V[15])
                    {goto Lbl2;}
                  else
                    if (V[14])
                      {goto Lbl92;}
                    else
                      return 0;
                else
                  if (V[15])
                    {goto Lbl34;}
                  else
                    if (V[14])
                      {goto Lbl114;}
                    else
                      return 0;
            else
              if (V[9])
                if (V[18])
                  {goto Lbl91;}
                else
                  if (V[15])
                    {statInv=(!statInv);goto Lbl30;}
                  else
                    if (V[14])
                      {goto Lbl38;}
                    else
                      if (V[6])
                        return 0;
                      else
                        {goto Lbl38;}
              else
                if (V[0])
                  return 0;
                else
                  {goto Lbl113;}
    else
      if (V[5])
        if (V[1])
          if (V[11])
            if (V[7])
              if (V[12])
                return 0;
              else
                if (V[3])
                  return 0;
                else
                  {goto Lbl40;}
            else
              if (V[12])
                if (V[18])
                  return 0;
                else
                  if (V[15])
                    return 0;
                  else
                    {goto Lbl20;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      return 0;
                    else
                      if (V[15])
                        return 0;
                      else
                        {goto Lbl41;}
                  else
                    if (V[18])
                      {goto Lbl94;}
                    else
                      if (V[15])
                        return 0;
                      else
                        {goto Lbl46;}
                else
                  if (V[9])
                    if (V[18])
                      return 0;
                    else
                      if (V[15])
                        {goto Lbl16;}
                      else
                        if (V[14])
                          if (V[20])
                            {goto Lbl20;}
                          else
                            Lbl115: if (V[21])
                              return 0;
                            else
                              {goto Lbl51;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl42;}
                  else
                    Lbl116: if (V[18])
                      {goto Lbl95;}
                    else
                      if (V[15])
                        {goto Lbl44;}
                      else
                        if (V[14])
                          Lbl117: if (V[17])
                            {goto Lbl49;}
                          else
                            {goto Lbl50;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl48;}
          else
            if (V[7])
              if (V[12])
                return 0;
              else
                if (V[3])
                  return 0;
                else
                  {goto Lbl61;}
            else
              if (V[12])
                if (V[18])
                  {goto Lbl93;}
                else
                  if (V[15])
                    return 0;
                  else
                    {goto Lbl57;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      {goto Lbl94;}
                    else
                      if (V[15])
                        return 0;
                      else
                        {goto Lbl63;}
                  else
                    if (V[18])
                      if (V[15])
                        return 0;
                      else
                        {goto Lbl72;}
                    else
                      if (V[15])
                        return 0;
                      else
                        {goto Lbl81;}
                else
                  if (V[9])
                    if (V[18])
                      {goto Lbl95;}
                    else
                      if (V[15])
                        {goto Lbl62;}
                      else
                        if (V[14])
                          if (V[20])
                            {goto Lbl57;}
                          else
                            if (V[21])
                              return !V[16];
                            else
                              {goto Lbl86;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl64;}
                  else
                    Lbl118: if (V[18])
                      Lbl119: if (V[15])
                        {goto Lbl71;}
                      else
                        if (V[14])
                          Lbl120: if (V[20])
                            {goto Lbl75;}
                          else
                            Lbl121: if (V[21])
                              return V[16];
                            else
                              {goto Lbl87;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl74;}
                    else
                      if (V[15])
                        {goto Lbl79;}
                      else
                        if (V[14])
                          Lbl122: if (V[17])
                            {goto Lbl84;}
                          else
                            {goto Lbl85;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl83;}
        else
          if (V[11])
            if (V[7])
              if (V[12])
                {goto Lbl39;}
              else
                if (V[3])
                  {goto Lbl40;}
                else
                  if (V[9])
                    if (V[18])
                      {goto Lbl15;}
                    else
                      if (V[15])
                        {goto Lbl45;}
                      else
                        if (V[14])
                          {goto Lbl47;}
                        else
                          {goto Lbl49;}
                  else
                    if (V[0])
                      return 0;
                    else
                      {goto Lbl43;}
            else
              if (V[12])
                if (V[18])
                  {goto Lbl93;}
                else
                  if (V[15])
                    {goto Lbl17;}
                  else
                    {goto Lbl50;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      {goto Lbl94;}
                    else
                      if (V[15])
                        {goto Lbl16;}
                      else
                        if (V[14])
                          if (V[20])
                            return 0;
                          else
                            {goto Lbl115;}
                        else
                          {goto Lbl49;}
                  else
                    if (V[18])
                      if (V[15])
                        {goto Lbl16;}
                      else
                        if (V[14])
                          {goto Lbl96;}
                        else
                          return 0;
                    else
                      if (V[15])
                        {goto Lbl44;}
                      else
                        if (V[14])
                          {goto Lbl117;}
                        else
                          return 0;
                else
                  if (V[9])
                    if (V[18])
                      {goto Lbl95;}
                    else
                      if (V[15])
                        {goto Lbl45;}
                      else
                        if (V[14])
                          {goto Lbl49;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl49;}
                  else
                    if (V[0])
                      return 0;
                    else
                      {goto Lbl116;}
          else
            if (V[7])
              Lbl123: if (V[12])
                {goto Lbl54;}
              else
                if (V[3])
                  {goto Lbl61;}
                else
                  if (V[9])
                    Lbl124: if (V[18])
                      {goto Lbl70;}
                    else
                      if (V[15])
                        {goto Lbl80;}
                      else
                        if (V[14])
                          {goto Lbl82;}
                        else
                          {goto Lbl84;}
                  else
                    if (V[0])
                      return 0;
                    else
                      {goto Lbl69;}
            else
              if (V[12])
                if (V[18])
                  if (V[15])
                    {goto Lbl10;}
                  else
                    {goto Lbl75;}
                else
                  if (V[15])
                    {goto Lbl55;}
                  else
                    {goto Lbl85;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      if (V[15])
                        {goto Lbl16;}
                      else
                        if (V[14])
                          if (V[20])
                            return 0;
                          else
                            Lbl125: if (V[21])
                              return 0;
                            else
                              {goto Lbl76;}
                        else
                          {goto Lbl74;}
                    else
                      if (V[15])
                        {goto Lbl62;}
                      else
                        if (V[14])
                          if (V[20])
                            return 0;
                          else
                            if (V[21])
                              return 0;
                            else
                              {goto Lbl86;}
                        else
                          {goto Lbl84;}
                  else
                    if (V[18])
                      if (V[15])
                        {goto Lbl71;}
                      else
                        if (V[14])
                          {goto Lbl120;}
                        else
                          return 0;
                    else
                      if (V[15])
                        {goto Lbl79;}
                      else
                        if (V[14])
                          {goto Lbl122;}
                        else
                          return 0;
                else
                  if (V[9])
                    if (V[18])
                      {goto Lbl119;}
                    else
                      if (V[15])
                        {goto Lbl80;}
                      else
                        if (V[14])
                          {goto Lbl84;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl84;}
                  else
                    if (V[0])
                      return 0;
                    else
                      {goto Lbl118;}
      else
        if (V[1])
          if (V[11])
            if (V[7])
              if (V[12])
                {goto Lbl39;}
              else
                if (V[3])
                  {goto Lbl40;}
                else
                  if (V[9])
                    if (V[18])
                      return 0;
                    else
                      if (V[15])
                        Lbl126: if (V[20])
                          {goto Lbl17;}
                        else
                          {statInv=(!statInv);goto Lbl10;}
                      else
                        if (V[14])
                          if (V[20])
                            {goto Lbl20;}
                          else
                            Lbl127: if (V[21])
                              return 0;
                            else
                              {goto Lbl59;}
                        else
                          if (V[20])
                            {goto Lbl125;}
                          else
                            if (V[21])
                              return 0;
                            else
                              {goto Lbl66;}
                  else
                    Lbl128: if (V[18])
                      {goto Lbl70;}
                    else
                      if (V[15])
                        Lbl129: if (V[17])
                          Lbl130: if (V[20])
                            {statInv=(!statInv);goto Lbl17;}
                          else
                            return V[21];
                        else
                          {statInv=(!statInv);goto Lbl17;}
                      else
                        if (V[14])
                          if (V[17])
                            Lbl131: if (V[20])
                              {goto Lbl73;}
                            else
                              {goto Lbl53;}
                          else
                            {goto Lbl73;}
                        else
                          if (V[17])
                            Lbl132: if (V[20])
                              {goto Lbl121;}
                            else
                              {goto Lbl88;}
                          else
                            if (V[20])
                              {goto Lbl121;}
                            else
                              {goto Lbl78;}
            else
              if (V[12])
                if (V[18])
                  return 0;
                else
                  if (V[15])
                    {goto Lbl17;}
                  else
                    {goto Lbl103;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      return 0;
                    else
                      if (V[15])
                        {goto Lbl16;}
                      else
                        if (V[14])
                          {goto Lbl102;}
                        else
                          Lbl133: if (V[20])
                            {goto Lbl103;}
                          else
                            if (V[21])
                              return 0;
                            else
                              {goto Lbl106;}
                  else
                    if (V[18])
                      {goto Lbl101;}
                    else
                      if (V[15])
                        {goto Lbl44;}
                      else
                        if (V[14])
                          if (V[17])
                            if (V[20])
                              {goto Lbl103;}
                            else
                              return 0;
                          else
                            {goto Lbl103;}
                        else
                          Lbl134: if (V[17])
                            Lbl135: if (V[20])
                              {goto Lbl110;}
                            else
                              Lbl136: if (V[21])
                                {goto Lbl99;}
                              else
                                return 0;
                          else
                            if (V[20])
                              {goto Lbl110;}
                            else
                              {goto Lbl105;}
                else
                  if (V[9])
                    if (V[18])
                      return 0;
                    else
                      if (V[15])
                        {goto Lbl126;}
                      else
                        if (V[14])
                          if (V[20])
                            {goto Lbl103;}
                          else
                            Lbl137: if (V[21])
                              return 0;
                            else
                              {goto Lbl111;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl133;}
                  else
                    Lbl138: if (V[18])
                      {goto Lbl108;}
                    else
                      if (V[15])
                        {goto Lbl129;}
                      else
                        if (V[14])
                          Lbl139: if (V[17])
                            {goto Lbl135;}
                          else
                            {goto Lbl110;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl134;}
          else
            if (V[7])
              if (V[12])
                {goto Lbl54;}
              else
                if (V[3])
                  {goto Lbl61;}
                else
                  if (V[9])
                    {goto Lbl124;}
                  else
                    return 0;
            else
              if (V[12])
                Lbl140: if (V[18])
                  {goto Lbl97;}
                else
                  if (V[15])
                    {goto Lbl55;}
                  else
                    Lbl141: if (V[21])
                      {goto Lbl99;}
                    else
                      Lbl142: if (V[8])
                        {goto Lbl86;}
                      else
                        if (V[19])
                          {goto Lbl52;}
                        else
                          if (V[16])
                            return V[24];
                          else
                            {goto Lbl60;}
              else
                if (V[3])
                  Lbl143: if (V[9])
                    if (V[18])
                      {goto Lbl101;}
                    else
                      if (V[15])
                        {goto Lbl62;}
                      else
                        if (V[14])
                          if (V[20])
                            return 0;
                          else
                            if (V[21])
                              return 0;
                            else
                              {goto Lbl142;}
                        else
                          Lbl144: if (V[20])
                            {goto Lbl141;}
                          else
                            if (V[21])
                              {goto Lbl99;}
                            else
                              Lbl145: if (V[8])
                                return 0;
                              else
                                if (V[19])
                                  return 0;
                                else
                                  if (V[16])
                                    return 0;
                                  else
                                    {goto Lbl68;}
                  else
                    if (V[18])
                      Lbl146: if (V[15])
                        {goto Lbl71;}
                      else
                        if (V[14])
                          {goto Lbl109;}
                        else
                          return 0;
                    else
                      if (V[15])
                        {goto Lbl79;}
                      else
                        if (V[14])
                          Lbl147: if (V[17])
                            Lbl148: if (V[20])
                              {goto Lbl141;}
                            else
                              {goto Lbl136;}
                          else
                            {goto Lbl141;}
                        else
                          Lbl149: if (V[17])
                            return 0;
                          else
                            if (V[20])
                              return 0;
                            else
                              if (V[21])
                                return 0;
                              else
                                {goto Lbl145;}
                else
                  if (V[9])
                    Lbl150: if (V[18])
                      {goto Lbl108;}
                    else
                      if (V[15])
                        {goto Lbl80;}
                      else
                        if (V[14])
                          {goto Lbl148;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl144;}
                  else
                    Lbl151: if (V[18])
                      return 0;
                    else
                      if (V[15])
                        return 0;
                      else
                        if (V[14])
                          return 0;
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl149;}
        else
          if (V[11])
            if (V[7])
              if (V[12])
                if (V[18])
                  {goto Lbl9;}
                else
                  if (V[15])
                    {statInv=(!statInv);goto Lbl17;}
                  else
                    {goto Lbl73;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      {goto Lbl15;}
                    else
                      if (V[15])
                        Lbl152: if (V[20])
                          return 0;
                        else
                          {statInv=(!statInv);goto Lbl10;}
                      else
                        if (V[14])
                          if (V[20])
                            return 0;
                          else
                            {goto Lbl127;}
                        else
                          if (V[20])
                            {goto Lbl73;}
                          else
                            if (V[21])
                              return !V[16];
                            else
                              {goto Lbl66;}
                  else
                    {goto Lbl128;}
                else
                  if (V[9])
                    if (V[18])
                      {goto Lbl70;}
                    else
                      if (V[15])
                        {goto Lbl130;}
                      else
                        if (V[14])
                          {goto Lbl131;}
                        else
                          {goto Lbl132;}
                  else
                    if (V[0])
                      return 0;
                    else
                      {goto Lbl128;}
            else
              if (V[12])
                if (V[18])
                  {goto Lbl97;}
                else
                  if (V[15])
                    {statInv=(!statInv);goto Lbl17;}
                  else
                    {goto Lbl110;}
              else
                if (V[3])
                  if (V[9])
                    if (V[18])
                      {goto Lbl101;}
                    else
                      if (V[15])
                        {goto Lbl152;}
                      else
                        if (V[14])
                          if (V[20])
                            return 0;
                          else
                            {goto Lbl137;}
                        else
                          {goto Lbl135;}
                  else
                    if (V[18])
                      {goto Lbl146;}
                    else
                      if (V[15])
                        {goto Lbl129;}
                      else
                        if (V[14])
                          {goto Lbl139;}
                        else
                          return 0;
                else
                  if (V[9])
                    if (V[18])
                      {goto Lbl108;}
                    else
                      if (V[15])
                        {goto Lbl130;}
                      else
                        if (V[14])
                          {goto Lbl135;}
                        else
                          if (V[6])
                            return 0;
                          else
                            {goto Lbl135;}
                  else
                    if (V[0])
                      return 0;
                    else
                      {goto Lbl138;}
          else
            if (V[7])
              if (V[2])
                return 0;
              else
                {goto Lbl123;}
            else
              if (V[2])
                if (V[12])
                  return 0;
                else
                  if (V[3])
                    return 0;
                  else
                    if (V[9])
                      return 0;
                    else
                      if (V[0])
                        return 0;
                      else
                        {goto Lbl151;}
              else
                if (V[12])
                  {goto Lbl140;}
                else
                  if (V[3])
                    {goto Lbl143;}
                  else
                    if (V[9])
                      {goto Lbl150;}
                    else
                      if (V[0])
                        {goto Lbl151;}
                      else
                        if (V[18])
                          {goto Lbl108;}
                        else
                          if (V[15])
                            {goto Lbl79;}
                          else
                            if (V[14])
                              {goto Lbl147;}
                            else
                              if (V[6])
                                {goto Lbl149;}
                              else
                                if (V[17])
                                  {goto Lbl144;}
                                else
                                  if (V[20])
                                    {goto Lbl141;}
                                  else
                                    if (V[21])
                                      {goto Lbl99;}
                                    else
                                      if (V[8])
                                        {goto Lbl65;}
                                      else
                                        if (V[19])
                                          {goto Lbl107;}
                                        else
                                          if (V[16])
                                            {goto Lbl8;}
                                          else
                                            if (V[22])
                                              {goto Lbl8;}
                                            else
                                              if (V[23])
                                                {goto Lbl14;}
                                              else
                                                {statInv=(!statInv);goto Lbl26;}

}





int _VT_IsSimple( int *V )
{
	int a=statProc( V );
	if ( statInv ) return( a ? 0 : 1 ); 
	else return( a ? 1 : 0 );
}
