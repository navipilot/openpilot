#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void live_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_9(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_12(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_35(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_32(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_33(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_H(double *in_vec, double *out_5015980593157513767);
void live_err_fun(double *nom_x, double *delta_x, double *out_1860005180627469095);
void live_inv_err_fun(double *nom_x, double *true_x, double *out_1902382254042256432);
void live_H_mod_fun(double *state, double *out_8394854480302840345);
void live_f_fun(double *state, double dt, double *out_8170256821180437676);
void live_F_fun(double *state, double dt, double *out_6156410520571324808);
void live_h_4(double *state, double *unused, double *out_4490985437783499760);
void live_H_4(double *state, double *unused, double *out_8374258154538532265);
void live_h_9(double *state, double *unused, double *out_4760385502323129369);
void live_H_9(double *state, double *unused, double *out_2785266983906571881);
void live_h_10(double *state, double *unused, double *out_1378114642409751093);
void live_H_10(double *state, double *unused, double *out_116278907094796186);
void live_h_12(double *state, double *unused, double *out_6738077689390158393);
void live_H_12(double *state, double *unused, double *out_5053029511139057556);
void live_h_35(double *state, double *unused, double *out_5147545742760872875);
void live_H_35(double *state, double *unused, double *out_6705823861798411975);
void live_h_32(double *state, double *unused, double *out_354434862447744727);
void live_H_32(double *state, double *unused, double *out_6574659076857671506);
void live_h_13(double *state, double *unused, double *out_5810751130783272681);
void live_H_13(double *state, double *unused, double *out_1515222710564246501);
void live_h_14(double *state, double *unused, double *out_4760385502323129369);
void live_H_14(double *state, double *unused, double *out_2785266983906571881);
void live_h_33(double *state, double *unused, double *out_1923132504004862756);
void live_H_33(double *state, double *unused, double *out_3555266857159554371);
void live_predict(double *in_x, double *in_P, double *in_Q, double dt);
}