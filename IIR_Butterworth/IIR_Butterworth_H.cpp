#include <iostream>
#include <math.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include<vector> 
#include <complex.h>
#include <algorithm>
#include "IIR_Butterworth.h"

//#define ARMA_DONT_USE_CXX11
#define ARMA_DONT_USE_CXX11_MUTEX
#include <armadillo>


#define PI 3.141592653589793

//Global variables
double fs = 2;
double u_f1;
double u_f2;
double Wn;
double Bw;

std::vector<std::complex<double>> ptemp;
std::complex<double> complex_real(1.0, 0.0);
std::complex<double> complex_imag(0.0, 1.0);
std::vector<std::complex<double>> p;

int temp_dim_arr_matr;
std::vector<std::vector<std::complex<double>> > a;
std::vector<std::complex<double>> b;
std::vector<std::complex<double>> c;

double d[1];
double t[1];

arma::cx_mat a_arma;
arma::cx_mat b_arma;
arma::cx_mat c_arma;
arma::cx_mat d_arma;

arma::cx_mat t1_arma;
arma::cx_mat t2_arma;
arma::cx_mat ad_arma;
arma::cx_mat bd_arma;
arma::cx_mat cd_arma;
arma::cx_mat dd_arma;

std::vector<double> num_filt;   //Vector where to temporarily save the numerator
std::vector<double> den_filt;   // Vector where to temporarily save the denumerator
std::vector<std::vector<double> > save_filt_coeff;  //Matrix where to save the numerator and denominator. First row is the numerator; second row is the denominator

using namespace IIR_B_F;

//Step 1: get analog, pre - warped frequencies
void IIR_Butterworth::freq_pre_wrapped(int type_filt, double Wnf_1, double Wnf_2)
{

    Bw = 0;

    switch (type_filt)
    {

        //Band-pass
    case 0:

        u_f1 = 2 * fs * tan(PI * Wnf_1 / fs);
        u_f2 = 2 * fs * tan(PI * Wnf_2 / fs);

        break;

        //Band-stop
    case 1:

        u_f1 = 2 * fs * tan(PI * Wnf_1 / fs);
        u_f2 = 2 * fs * tan(PI * Wnf_2 / fs);


        break;

        //High-pass
    case 2:

        u_f1 = 2 * fs * tan(PI * Wnf_1 / fs);

        break;

        //Low-pass
    case 3:

        u_f2 = 2 * fs * tan(PI * Wnf_2 / fs);

        break;

    }
}


//Step 2: convert to low-pass prototype estimate
void IIR_Butterworth::Wn_f1_Wn_f2(int type_filt, double u_f1, double u_f2)
{

    switch (type_filt)
    {

        //Band-pass
    case 0:
        Bw = u_f2 - u_f1;
        Wn = sqrt(u_f1 * u_f2);

        break;

        //Band-stop
    case 1:

        Bw = u_f2 - u_f1;
        Wn = sqrt(u_f1 * u_f2);

        break;

        //Low-pass
    case 2:

        Wn = u_f1;

        break;

        //High-pass
    case 3:

        Wn = u_f2;

        break;

    }
}



//Step 3: Get N - th order Butterworth analog lowpass prototype
void IIR_Butterworth::buttap(int order_filt)
{
    double order_filt_exp = (double)order_filt;
    int temp_length_vec = 0;
    int kkk = 1;
    do {

        temp_length_vec++;
        kkk += 2;

    } while (kkk <= order_filt - 1);

    //Initialize the vector "ptemp" 
    for (int i = 0; i < temp_length_vec; i++)
    {

        ptemp.push_back(0);

    }
    
    int track_cell = 0;
    for (double kk = 0; kk < (double)order_filt - 1; kk += 2)
    {

        ptemp.at(track_cell) = exp(complex_imag * ((PI * (kk + 1)) / (2 * order_filt_exp) + PI / 2));

        track_cell++;

    }

    //Initialize the vector "p"
    for (int i = 0; i < order_filt; i++)

    {

        p.push_back(0);

    }

   
    double temp_rem = remainder(order_filt, 2);

    if (temp_rem != 0)
    {

        for (int kk = 0; kk < order_filt; kk++)
        {

            if (kk < order_filt - 1)
            {

                p.at(kk) = complex_imag;

            }

            else
            {

                p.at(kk) = -complex_real;

            }


        }

    }

    else
    {

        for (int kk = 0; kk < order_filt; kk++)
        {

            p.at(kk) = complex_imag;

        }


    }

    if (order_filt > 1)
    {
        track_cell = 0;
        for (int kk = 0; kk < temp_length_vec * 2; kk += 2)
        {

            p.at(kk) = ptemp.at(track_cell);
            p.at(kk + 1) = conj(ptemp.at(track_cell));

            track_cell++;

        }
    }

}



//Step 4: Transform to state-space
//Intermidiate step: calculate the coefficients of the polynomial (based on Matlab code)
std::vector<std::complex<double>> IIR_Butterworth::poly(std::vector<std::complex<double>> temp_array_poly, int col_poly)
{
    std::vector<std::complex<double>> coeff_pol_f(col_poly + 1);
    coeff_pol_f.at(0) = 1;

    for (int ll = 0; ll < col_poly; ll++)
    {

        int yy = 0;

        do
        {

            coeff_pol_f.at(ll + 1 - yy) = coeff_pol_f.at(ll + 1 - yy) - temp_array_poly.at(ll) * coeff_pol_f.at(ll - yy);
            yy++;

        } while (yy <= ll);

    }

    return coeff_pol_f;
}


//Calculate the factorial of the given number
int IIR_Butterworth::factorial(int i)
{

    if (i <= 1) 
    {

        return 1;

    }

    return i * factorial(i - 1);

}



//Method to calculate all the possible combinations. Code taken and slightly modified from: 
//http://rosettacode.org/wiki/Combinations#C.2B.2B
std::vector<std::vector<int> > IIR_Butterworth::combination_method(int N, int K, int comb_n)
{
    
    int rows_f = 0;
    
    std::vector<std::vector<int> > matrix_comb_f;

        std::vector<int> temp_v;

        for (int ff = 0; ff < K; ff++)
        {

            temp_v.push_back(0);

        }

        for (int hh = 0; hh < comb_n; hh++)
        {

            matrix_comb_f.push_back(temp_v);

        }

    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    int col_f;
    do {

        col_f = 0;

        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {

            if (bitmask[i])

            {
                
               
                matrix_comb_f[rows_f][col_f] = i;
               
                col_f++;

            }
        }
        
        rows_f++;

    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

    return matrix_comb_f;

}



//Intermidiate step: calculate the coefficients of the polynomial (Bernard Brooks' paper (2016))
std::vector<std::complex<double>> IIR_Butterworth::char_poly(arma::cx_mat temp_matr_poly, int row_col)
{
    
    std::vector<std::complex<double>> coeff_pol_ff(row_col + 1);
    arma::cx_mat temp_val = arma::zeros <arma::cx_mat>(1,1);
    double num_det;
    arma::cx_mat temp_matr = arma::zeros <arma::cx_mat>(row_col, row_col);
    
    std::vector<std::vector<int> > matrix_comb;

    for (int kk = 0; kk < row_col + 1; kk++)

    {

        if (kk == 0)
        {

            coeff_pol_ff[row_col - kk] = pow(-1, row_col) * arma::det(temp_matr_poly);

        }

        else
        {

            temp_val(0, 0) = 0;


            num_det = (double)factorial(row_col) / (double)(factorial(row_col - kk) * (double)factorial(kk));  //Calculate the number of combinations   


        //Catching the situation where the denominator is zero. Assign the default value of 10^10 to the denominator
            if (isnan(num_det))
            {

                for (int ff = 0; ff < row_col + 1; ff++)
                {

                    coeff_pol_ff[ff] = pow(10, 10);

                }

                break;

            }

            if (num_det < 1)    //Handling the situation where numerical overflow generates a negative or zero value for the num_det because of the instability of the filter
            {

                for (int ff = 0; ff < row_col + 1; ff++)
                {

                    coeff_pol_ff[ff] = pow(10, 10);

                }

                break;

            }

            std::vector<int> temp_v;

            for (int ff = 0; ff < num_det; ff++)
            {

                temp_v.push_back(0);

            }

            for (int hh = 0; hh < num_det; hh++)
            {

                matrix_comb.push_back(temp_v);

            }

            if (num_det - (int)num_det == 0)
            {
                // Generate the combinations 
                matrix_comb = combination_method(row_col, kk, (int)num_det);

                for (int mm = 0; mm < num_det; mm++)

                {

                    temp_matr = temp_matr_poly;

                    for (int pp = 0; pp < row_col; pp++)
                    {

                        temp_matr(matrix_comb[mm][0], pp) = 0;
                        temp_matr(pp, matrix_comb[mm][0]) = 0;
                        temp_matr(matrix_comb[mm][0], matrix_comb[mm][0]) = -1;

                    }

                    for (int nn = 1; nn < kk; nn++)
                    {

                        for (int pp = 0; pp < row_col; pp++)
                        {

                            temp_matr(matrix_comb[mm][nn], pp) = 0;
                            temp_matr(pp, matrix_comb[mm][nn]) = 0;
                            temp_matr(matrix_comb[mm][nn], matrix_comb[mm][nn]) = -1;

                        }

                    }

                    temp_val(0, 0) += det(temp_matr);

                }

                coeff_pol_ff[row_col - kk] = pow(-1, row_col) * temp_val(0, 0);


            }

            else
            {

                for (int ff = 0; ff < row_col + 1; ff++)
                {

                    coeff_pol_ff[ff] = pow(10, 10);

                }

                break;

            }

        }

    }

    
    return coeff_pol_ff;
    
}



//Step 4: Transform to state-space
void IIR_Butterworth::zp2ss(int order_filt)
{

    //Order the pairs of complex conjugate. The pairs with the smallest real part come first. Within pairs, the ones with the negative imaginary part comes first    
    std::complex<double> temp_max;

    //Using the selection sort algorithm to order based on the real part
    double order_filt_d = (double)order_filt;
    double temp_rem;
    if (remainder(order_filt, 2) == 0)
    {

        temp_rem = order_filt_d;

    }

    else
    {

        temp_rem = order_filt_d - 1;

    }

    int min_real;
    for (int kk = 0; kk < temp_rem - 1; kk++)
    {
        min_real = kk;

        for (int jj = kk + 1; jj < temp_rem; jj++)
        {
            if (real(p[jj]) < real(p[min_real]))
            {

                min_real = jj;

                temp_max = p[kk];
                p[kk] = p[min_real];
                p[min_real] = temp_max;

            }
        }
    }

    //Using the selection sort algorithm to order the values based on the imaginary part
    for (int kk = 0; kk < temp_rem - 1; kk += 2)
    {
        min_real = kk;


        if (imag(p[kk]) > imag(p[kk + 1]))
        {

            min_real = kk + 1;

            temp_max = p[kk];
            p[kk] = p[min_real];
            p[min_real] = temp_max;

        }
    }


    // Initialize state - space matrices for running series
    d[0] = 1;

    int track_index = 1;

    // Take care of any left over unmatched pole pairs.
        // H(s) = 1 / (s ^ 2 + den(2)s + den(3))
    std::vector<std::complex<double>> temp_poly_p;

    double wn = 1;

    if (order_filt > 1)
    {

        double b1[2] = { 1,0 };
        double c1[2] = { 0,1 };
        double d1 = 0;


        temp_rem = remainder(order_filt, 2);
        int order_filt_temp;
        int dim_matr;
        std::vector<std::vector<std::complex<double>> > temp_matrix_a;    //Temporary matrix where to save the coefficients at each interaction
        int coeff_numb = 3;

        if (temp_rem == 0)
        {

            order_filt_temp = order_filt;
            dim_matr = order_filt_temp / 2;

                std::vector<std::complex<double>> temp_v;

                for (int ll = 0; ll < coeff_numb; ll++)
                {

                    temp_v.push_back(0);

                }

                for (int kk = 0; kk < dim_matr; kk++)
                {

                    temp_matrix_a.push_back(temp_v);
                
                }

        }

        else
        {

            order_filt_temp = order_filt - 1;
            dim_matr = order_filt_temp / 2;

                std::vector<std::complex<double>> temp_v;

                for (int ll = 0; ll < coeff_numb; ll++)
                {

                    temp_v.push_back(0);

                }

                for (int kk = 0; kk < dim_matr; kk++)
                {

                    temp_matrix_a.push_back(temp_v);
            
                }

        }

        int track_cycles = 0;
        int temp_val_pos_check;
        int dim_poly_p = 2;
        
        for (int i = 0; i < dim_poly_p; i++)
        {

            temp_poly_p.push_back(0);

        }

        temp_dim_arr_matr = dim_poly_p + (order_filt % 2);

        while (track_index < order_filt_temp)
        {

            for (int rr = track_index - 1; rr < track_index + 1; rr++)
            {

                temp_poly_p[rr - track_index + 1] = p[rr];

            }

            std::vector<std::complex<double>> coeff_pol(dim_poly_p + 1);

            coeff_pol = poly(temp_poly_p, dim_poly_p);


            for (int qq = 0; qq < coeff_numb; qq++)
            {

                temp_matrix_a[track_cycles][qq] = -real(coeff_pol[qq]);

            }

            //Update the state-space arrays/matrix
            track_cycles += 1;

                std::vector<std::complex<double>> temp_v;

                for (int ll = 0; ll < temp_dim_arr_matr; ll++)
                {

                    temp_v.push_back(0);

                }

                for (int kk = 0; kk < temp_dim_arr_matr; kk++)
                {

                    a.push_back(temp_v);
            
                }

            int track_index_coeff = 0;

            if (remainder(order_filt, 2) == 0)
            {
                ///////////////////////////////////////////////////////////////////////////////////////////////
                //Even number of poles
                for (int kk = 0; kk < temp_dim_arr_matr; kk++)
                {

                    for (int gg = 0; gg < temp_dim_arr_matr; gg++)

                    {

                        temp_val_pos_check = kk - gg;

                        switch (temp_val_pos_check)
                        {
                        case 1:

                            a[kk][gg] = 1;

                            break;


                        case 0:

                            if ((remainder(kk + 1, 2) != 0) || (kk == 0))
                            {

                                a[kk][gg] = temp_matrix_a[track_index_coeff][1];

                            }

                            else
                            {

                                a[kk][gg] = 0;

                            }

                            break;

                        case -1:

                            if ((remainder(kk + 1, 2) != 0) || (kk == 0))
                            {

                                a[kk][gg] = temp_matrix_a[track_index_coeff][2];
                                track_index_coeff++;

                            }

                            else
                            {

                                a[kk][gg] = 0;

                            }
                            break;

                        default:

                            a[kk][gg] = 0;

                            break;


                        }

                    }

                }
                ///////////////////////////////////////////////////////////////////////////////////////////////
            }

            else
            {
                ///////////////////////////////////////////////////////////////////////////////////////////////
                //Odd number of poles
                for (int kk = 0; kk < temp_dim_arr_matr; kk++)
                {

                    for (int gg = 0; gg < temp_dim_arr_matr; gg++)

                    {

                        temp_val_pos_check = kk - gg;

                        switch (temp_val_pos_check)
                        {
                        case 1:

                            a[kk][gg] = 1;

                            break;


                        case 0:

                            if (kk == 0)
                            {

                                a[kk][gg] = -1;

                            }

                            else
                            {

                                if ((remainder(kk + 1, 2) == 0))
                                {

                                    a[kk][gg] = temp_matrix_a[track_index_coeff][1];

                                }

                                else
                                {

                                    a[kk][gg] = 0;

                                }
                            }

                            break;

                        case -1:

                            if ((remainder(kk + 1, 2) == 0))
                            {

                                a[kk][gg] = temp_matrix_a[track_index_coeff][2];
                                track_index_coeff++;

                            }

                            else
                            {

                                a[kk][gg] = 0;

                            }
                            break;

                        default:

                            a[kk][gg] = 0;

                            break;


                        }

                    }

                }

            }
             ///////////////////////////////////////////////////////////////////////////////////////////////

             //Initialize the vectors "b" and "c"
            for (int i = 0; i < temp_dim_arr_matr; i++)
            {

                b.push_back(0);
                c.push_back(0);

            }

           
            for (int kk = 0; kk < temp_dim_arr_matr; kk++)
            {

                if (kk == 0)
                {

                    b[kk] = 1;

                }

                else
                {

                    b[kk] = 0;

                }

            }

            for (int kk = 0; kk < temp_dim_arr_matr; kk++)
            {

                if (kk == temp_dim_arr_matr - 1)
                {

                    c[kk] = 1;

                }

                else
                {

                    c[kk] = 0;

                }

            }

            track_index += 2;

            if (track_index < order_filt_temp)
            {

                //Clean up the matrix "a" and the arrays "b" and "c", so they can be re-initialized
                a.erase(a.begin(), a.begin() + temp_dim_arr_matr);
                b.erase(b.begin(), b.begin() + temp_dim_arr_matr);
                c.erase(c.begin(), c.begin() + temp_dim_arr_matr);

            }

            dim_matr += 2;
            temp_dim_arr_matr += 2;

        }       

    }

    else
    {


        std::vector<std::complex<double>> temp_v;
        temp_v.push_back(0);
        a.push_back(temp_v);
        a[0][0] = p[0];

        b.push_back(1);
        c.push_back(1);

    }
    
    d[0] = 0;


}


//Extract the coefficients of the band pass filter
std::vector<std::vector<double> > IIR_Butterworth::lp2bp(double W_f1, double W_f2, int order_filt)
{

    //Clean up the global variables for a new analysis
    if (save_filt_coeff.size() > 0)
    {

        save_filt_coeff.erase(save_filt_coeff.begin(), save_filt_coeff.begin() + save_filt_coeff.size());
        ptemp.erase(ptemp.begin(), ptemp.begin() + ptemp.size());
        p.erase(p.begin(), p.begin() + p.size());

        a.erase(a.begin(), a.begin() + a.size());
        b.erase(b.begin(), b.begin() + b.size());
        c.erase(c.begin(), c.begin() + c.size());

        num_filt.erase(num_filt.begin(), num_filt.begin() + num_filt.size());
        den_filt.erase(den_filt.begin(), den_filt.begin() + den_filt.size());

    }

        std::vector<double> temp_v;

        for (int ff = 0; ff < 2*order_filt + 1; ff++)
        {

            temp_v.push_back(0);

        }

        for (int hh = 0; hh < 2; hh++)
        {

            save_filt_coeff.push_back(temp_v);
        
        }

    int type_filt = 0;

    //Step 1: get analog, pre - warped frequencies
    freq_pre_wrapped(type_filt, W_f1, W_f2); 

    
    //Step 2: convert to low-pass prototype estimate
    Wn_f1_Wn_f2(type_filt, u_f1, u_f2);

    
    //Step 3: Get N - th order Butterworth analog lowpass prototype
    buttap(order_filt);
    
    //Step 4: Transform to state-space
    zp2ss(order_filt);

    
    if (order_filt > 1)
    {

        temp_dim_arr_matr -= 2;

    }

    else
    {

        temp_dim_arr_matr = order_filt;

    }

    
    //Copy the values of the matrix/arrays "arma" matrix/array in order to compute the pseudo-inverse of the matrix and other matrix operations
    a_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 2 * temp_dim_arr_matr);
    arma::cx_mat a_arma_p_eye = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    a_arma_p_eye = a_arma_p_eye.eye();
    arma::cx_mat a_arma_n_eye = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    a_arma_n_eye = -a_arma_p_eye.eye();
    b_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 1);
    c_arma = arma::zeros <arma::cx_mat>(1, 2 * temp_dim_arr_matr);
    d_arma = arma::zeros <arma::cx_mat>(1, 1);

    
    double q = Wn / Bw;
    for (int kk = 0; kk < 2 * temp_dim_arr_matr; kk++)
    {
        if (kk < temp_dim_arr_matr)
        {

            b_arma(kk, 0) = b[kk] * Wn / q;
            c_arma(0, kk) = c[kk];

        }

        for (int ll = 0; ll < 2 * temp_dim_arr_matr; ll++)
        {

            if (kk < temp_dim_arr_matr)
            {

                if (ll < temp_dim_arr_matr)

                {

                    a_arma(kk, ll) = Wn * a[kk][ll] / q;

                }

                else
                {

                    a_arma(kk, ll) = Wn * a_arma_p_eye(kk, ll - temp_dim_arr_matr);

                }
            }

            else
            {
                if (ll < temp_dim_arr_matr)
                {

                    a_arma(kk, ll) = Wn * a_arma_n_eye(kk - temp_dim_arr_matr, ll);

                }
            }

        }

    }

    d_arma = d_arma;
    

    //Step 5: Use Bilinear transformation to find discrete equivalent
    bilinear(a_arma, b_arma, c_arma, d_arma, fs, type_filt);
    
    //Step 6: Transform to zero-pole-gain and polynomial forms
    zero_pole_gain(ad_arma, type_filt, order_filt, Wn, Bw);

    return save_filt_coeff;

}

   
//Extract the coefficients of the band stop filter
std::vector<std::vector<double> > IIR_Butterworth::lp2bs(double W_f1, double W_f2, int order_filt)
{

    //Clean up the global variables for a new analysis
    if (save_filt_coeff.size() > 0)
    {

        save_filt_coeff.erase(save_filt_coeff.begin(), save_filt_coeff.begin() + save_filt_coeff.size());
        ptemp.erase(ptemp.begin(), ptemp.begin() + ptemp.size());
        p.erase(p.begin(), p.begin() + p.size());

        a.erase(a.begin(), a.begin() + a.size());
        b.erase(b.begin(), b.begin() + b.size());
        c.erase(c.begin(), c.begin() + c.size());

        num_filt.erase(num_filt.begin(), num_filt.begin() + num_filt.size());
        den_filt.erase(den_filt.begin(), den_filt.begin() + den_filt.size());

    }


        std::vector<double> temp_v;

        for (int ff = 0; ff < 2 * order_filt + 1; ff++)
        {

            temp_v.push_back(0);

        }

        for (int hh = 0; hh < 2; hh++)
        {

            save_filt_coeff.push_back(temp_v);

        }

    int type_filt = 1;

    //Step 1: get analog, pre - warped frequencies
    freq_pre_wrapped(type_filt, W_f1, W_f2);

    //Step 2: convert to low-pass prototype estimate
    Wn_f1_Wn_f2(type_filt, u_f1, u_f2);

    //Step 3: Get N - th order Butterworth analog lowpass prototype
    buttap(order_filt);

    //Step 4: Transform to state-space
    zp2ss(order_filt);

    if (order_filt > 1)
    {

        temp_dim_arr_matr -= 2;

    }

    else
    {

        temp_dim_arr_matr = order_filt;

    }

    //Copy the values of the matrix/arrays "arma" matrix/array in order to compute the pseudo-inverse of the matrix and other matrix operations
    a_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 2 * temp_dim_arr_matr);
    arma::cx_mat a_arma_p_eye = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    a_arma_p_eye = a_arma_p_eye.eye();
    arma::cx_mat a_arma_n_eye = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    a_arma_n_eye = -a_arma_p_eye.eye();
    b_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 1);
    c_arma = arma::zeros <arma::cx_mat>(1, 2 * temp_dim_arr_matr);
    d_arma = arma::zeros <arma::cx_mat>(1, 1);


    arma::cx_mat a_arma_pinv = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    arma::cx_mat b_arma_temp = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, 1);
    arma::cx_mat c_arma_temp = arma::zeros <arma::cx_mat>(1, temp_dim_arr_matr);

    arma::cx_mat a_b_arma_temp = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, 1);
    arma::cx_mat c_a_arma_temp = arma::zeros <arma::cx_mat>(1, temp_dim_arr_matr);

    for (int kk = 0; kk < temp_dim_arr_matr; kk++)
    {
        b_arma_temp(kk, 0) = b[kk];
        c_arma_temp(0, kk) = c[kk];

        for (int ll = 0; ll < temp_dim_arr_matr; ll++)
        {

            a_arma_pinv(kk, ll) = a[kk][ll];

        }

    }

    double q = Wn / Bw;

    try
    {

        a_arma_pinv = (Wn / q) * arma::pinv(a_arma_pinv);

    }

    catch (std::runtime_error)
    {

        for (int kk = 0; kk < 2; kk++)
        {

            for (int hh = 0; hh < 2 * order_filt + 1; hh++)
            {

                save_filt_coeff[kk][hh] = pow(10, 10);

            }
        

        }
        
        return save_filt_coeff;

    }


    a_b_arma_temp = (Wn) * (a_arma_pinv * b_arma_temp) / q;
    c_a_arma_temp = c_arma_temp * a_arma_pinv;

    for (int kk = 0; kk < 2 * temp_dim_arr_matr; kk++)
    {
        if (kk < temp_dim_arr_matr)
        {

            b_arma(kk, 0) = -a_b_arma_temp(kk, 0);
            c_arma(0, kk) = c_a_arma_temp(0, kk);

        }

        for (int ll = 0; ll < 2 * temp_dim_arr_matr; ll++)
        {

            if (kk < temp_dim_arr_matr)
            {

                if (ll < temp_dim_arr_matr)

                {

                    a_arma(kk, ll) = a_arma_pinv(kk, ll);

                }

                else
                {

                    a_arma(kk, ll) = Wn * a_arma_p_eye(kk, ll - temp_dim_arr_matr);

                }
            }

            else
            {
                if (ll < temp_dim_arr_matr)
                {

                    a_arma(kk, ll) = Wn * a_arma_n_eye(kk - temp_dim_arr_matr, ll);

                }
            }

        }

    }

    d_arma = d[0] + c_a_arma_temp * b_arma_temp;

    //Step 5: Use Bilinear transformation to find discrete equivalent
    bilinear(a_arma, b_arma, c_arma, d_arma, fs, type_filt);

    //Step 6: Transform to zero-pole-gain and polynomial forms
    zero_pole_gain(ad_arma, type_filt, order_filt, Wn, Bw);

    return save_filt_coeff;

}

//Extract the coefficients of the high pass filter
std::vector<std::vector<double> > IIR_Butterworth::lp2hp(double W_f2, int order_filt)
{

    //Clean up the global variables for a new analysis
    if (save_filt_coeff.size() > 0)
    {

        save_filt_coeff.erase(save_filt_coeff.begin(), save_filt_coeff.begin() + save_filt_coeff.size());
        ptemp.erase(ptemp.begin(), ptemp.begin() + ptemp.size());
        p.erase(p.begin(), p.begin() + p.size());

        a.erase(a.begin(), a.begin() + a.size());
        b.erase(b.begin(), b.begin() + b.size());
        c.erase(c.begin(), c.begin() + c.size());

        num_filt.erase(num_filt.begin(), num_filt.begin() + num_filt.size());
        den_filt.erase(den_filt.begin(), den_filt.begin() + den_filt.size());

    }

    std::vector<double> temp_v;

    for (int ff = 0; ff < 2 * order_filt; ff++)
    {

        temp_v.push_back(0);

    }

    for (int hh = 0; hh < 2; hh++)
    {

        save_filt_coeff.push_back(temp_v);

    }

    int type_filt = 2;

    //Step 1: get analog, pre - warped frequencies
    freq_pre_wrapped(type_filt, W_f2, 0);

    //Step 2: convert to low-pass prototype estimate
    Wn_f1_Wn_f2(type_filt, u_f1, u_f2);

    //Step 3: Get N - th order Butterworth analog lowpass prototype
    buttap(order_filt);

    //Step 4: Transform to state-space
    zp2ss(order_filt);

    if (order_filt > 1)
    {

        temp_dim_arr_matr -= 2;

    }

    else
    {

        temp_dim_arr_matr = order_filt;

    }

    //Copy the values of the matrix/arrays "arma" matrix/array in order to compute the pseudo-inverse of the matrix and other matrix operations
    a_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    b_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, 1);
    c_arma = arma::zeros <arma::cx_mat>(1, temp_dim_arr_matr);
    d_arma = arma::zeros <arma::cx_mat>(1, 1);

    for (int kk = 0; kk < temp_dim_arr_matr; kk++)
    {
        b_arma(kk, 0) = b[kk];
        c_arma(0, kk) = c[kk];

        for (int ll = 0; ll < temp_dim_arr_matr; ll++)
        {

            a_arma(kk, ll) = a[kk][ll];

        }

    }

    try
    {

        d_arma = d_arma - (c_arma * arma::pinv(a_arma)) * b_arma;

    }

    catch (std::runtime_error)
    {

        for (int kk = 0; kk < 2; kk++)
        {

            for (int hh = 0; hh < order_filt + 1; hh++)
            {

                save_filt_coeff[kk][hh] = pow(10, 10);

            }


        }

        return save_filt_coeff;

    }

    c_arma = c_arma * arma::pinv(a_arma);
    b_arma = -Wn * arma::pinv(a_arma) * b_arma;
    a_arma = Wn * arma::pinv(a_arma);


    //Step 5: Use Bilinear transformation to find discrete equivalent
    bilinear(a_arma, b_arma, c_arma, d_arma, fs, type_filt);

    //Step 6: Transform to zero-pole-gain and polynomial forms
    zero_pole_gain(ad_arma, type_filt, order_filt, Wn, Bw);

    return save_filt_coeff;

}


//Extract the coefficients of the low pass filter
std::vector<std::vector<double> > IIR_Butterworth::lp2lp(double W_f1, int order_filt)
{

    //Clean up the global variables for a new analysis
    if (save_filt_coeff.size() > 0)
    {

        save_filt_coeff.erase(save_filt_coeff.begin(), save_filt_coeff.begin() + save_filt_coeff.size());
        ptemp.erase(ptemp.begin(), ptemp.begin() + ptemp.size());
        p.erase(p.begin(), p.begin() + p.size());

        a.erase(a.begin(), a.begin() + a.size());
        b.erase(b.begin(), b.begin() + b.size());
        c.erase(c.begin(), c.begin() + c.size());

        num_filt.erase(num_filt.begin(), num_filt.begin() + num_filt.size());
        den_filt.erase(den_filt.begin(), den_filt.begin() + den_filt.size());

    }


        std::vector<double> temp_v;

        for (int ff = 0; ff < 2 * order_filt; ff++)
        {

            temp_v.push_back(0);

        }

        for (int hh = 0; hh < 2; hh++)
        {

            save_filt_coeff.push_back(temp_v);

        }

    int type_filt = 3;

    //Step 1: get analog, pre - warped frequencies
    freq_pre_wrapped(type_filt, 0, W_f1);

    //Step 2: convert to low-pass prototype estimate
    Wn_f1_Wn_f2(type_filt, u_f1, u_f2);

    //Step 3: Get N - th order Butterworth analog lowpass prototype
    buttap(order_filt);

    //Step 4: Transform to state-space
    zp2ss(order_filt);

    if (order_filt > 1)
    {

        temp_dim_arr_matr -= 2;

    }

    else
    {

        temp_dim_arr_matr = order_filt;

    }

    //Copy the values of the matrix/arrays "arma" matrix/array in order to compute the pseudo-inverse of the matrix and other matrix operations
    a_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
    b_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, 1);
    c_arma = arma::zeros <arma::cx_mat>(1, temp_dim_arr_matr);
    d_arma = arma::zeros <arma::cx_mat>(1, 1);

    for (int kk = 0; kk < temp_dim_arr_matr; kk++)
    {
        b_arma(kk, 0) = b[kk];
        c_arma(0, kk) = c[kk];

        for (int ll = 0; ll < temp_dim_arr_matr; ll++)
        {

            a_arma(kk, ll) = a[kk][ll];

        }

    }

    d_arma = d_arma;
    c_arma = c_arma;
    b_arma = Wn * b_arma;
    a_arma = Wn * a_arma;


    //Step 5: Use Bilinear transformation to find discrete equivalent
    bilinear(a_arma, b_arma, c_arma, d_arma, fs, type_filt);

    //Step 6: Transform to zero-pole-gain and polynomial forms
    zero_pole_gain(ad_arma, type_filt, order_filt, Wn, Bw);

    return save_filt_coeff;

}




//Step 5: Use Bilinear transformation to find discrete equivalent
void IIR_Butterworth::bilinear(arma::cx_mat a_arma_f, arma::cx_mat b_arma_f, arma::cx_mat c_arma_f, arma::cx_mat d_arma_f, double fs_f, int type_filt_f)
{

    double t_arma;
    double r_arma;

    if (type_filt_f > 1)
    {
        t1_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
        t2_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
        ad_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, temp_dim_arr_matr);
        bd_arma = arma::zeros <arma::cx_mat>(temp_dim_arr_matr, 1);
        cd_arma = arma::zeros <arma::cx_mat>(1, temp_dim_arr_matr);
        dd_arma = arma::zeros <arma::cx_mat>(1, 1);
    }

    else
    {

        t1_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 2 * temp_dim_arr_matr);
        t2_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 2 * temp_dim_arr_matr);
        ad_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 2 * temp_dim_arr_matr);
        bd_arma = arma::zeros <arma::cx_mat>(2 * temp_dim_arr_matr, 1);
        cd_arma = arma::zeros <arma::cx_mat>(1, 2 * temp_dim_arr_matr);
        dd_arma = arma::zeros <arma::cx_mat>(1, 1);

    }

    try
    {
        t_arma = (1 / fs_f);
        r_arma = sqrt(t_arma);
        t1_arma = t1_arma.eye() + a_arma_f * t_arma * 0.5; //t1_arma.eye() 
        t2_arma = t2_arma.eye() - a_arma_f * t_arma * 0.5;
        ad_arma = t1_arma * arma::pinv(t2_arma);
        bd_arma = (t_arma / r_arma) * arma::solve(t2_arma, b_arma_f);
        cd_arma = (r_arma * c_arma_f) * arma::pinv(t2_arma);
        dd_arma = (c_arma_f * arma::pinv(t2_arma)) * b_arma_f * (t_arma / 2) + d_arma_f;
    }

    catch (std::runtime_error)
    {



    }
}



//Step 6: Transform to zero-pole-gain and polynomial forms
void IIR_Butterworth::zero_pole_gain(arma::cx_mat a_arma_f, int type_filt_f, int order_filt_f, double Wn_f_f, double Bw_f)
{

    int dim_array;

    if (type_filt_f > 1)
    {

        //Initialize the vectors "num_filt" and "den_filt"
        for (int i = 0; i < order_filt_f + 1; i++)
        {

            num_filt.push_back(0);
            den_filt.push_back(0);

        }


        dim_array = temp_dim_arr_matr;

    }

    else
    {

        //Initialize the vectors "num_filt" and "den_filt"
        for (int i = 0; i < 2 * order_filt_f + 1; i++)
        {

            num_filt.push_back(0);
            den_filt.push_back(0);

        }


        dim_array = 2 * temp_dim_arr_matr;

    }

    //Extract the coefficients of the denumerator
    std::vector<std::complex<double>> coeff_pol(temp_dim_arr_matr + 1);

    if (type_filt_f > 1)

    {

        coeff_pol = char_poly(a_arma_f, temp_dim_arr_matr);

    }

    else
    {

        coeff_pol = char_poly(a_arma_f, 2 * temp_dim_arr_matr);

    }


    for (int qq = 0; qq < dim_array + 1; qq++)
    {

        den_filt[qq] = real(coeff_pol[qq]);
        save_filt_coeff[1][qq] = den_filt[qq];

    }


    //Extract the coefficients of the denominator
    double w;
    Wn = 2 * std::atan2(Wn, 4);
    std::vector<std::complex<double>> r;

    switch (type_filt_f)
    {

    case 0: // band-pass

        for (int i = 0; i <= dim_array; i++)
        {

            r.push_back(0);

        }

        for (int kk = 0; kk < dim_array; kk++)
        {

            if (kk < temp_dim_arr_matr)
            {

                r[kk] = 1;

            }

            else
            {

                r[kk] = -1;

            }

        }

        w = Wn;

        break;

    case 1: // band-stop

        for (int i = 0; i <= dim_array; i++)
        {

            r.push_back(0);

        }


        for (int kk = 0; kk < dim_array; kk++)
        {

            r[kk] = exp(complex_imag * Wn * pow(-1, kk));

        }
        w = 0;

        break;

    case 2: //high-pass

        for (int i = 0; i <= dim_array; i++)
        {

            r.push_back(0);

        }

        for (int kk = 0; kk < dim_array; kk++)
        {

            r[kk] = 1;

        }

        w = PI;
        break;

    case 3: // low-pass

        for (int i = 0; i <= dim_array; i++)
        {

            r.push_back(0);

        }

        for (int kk = 0; kk < dim_array; kk++)
        {

            r[kk] = -1;

        }
        w = 0;
        break;

    default:
        for (int i = 0; i <= dim_array; i++)
        {

            r.push_back(0);

        }

    }

    std::vector<std::complex<double>> coeff_pol_num(dim_array + 1);

    coeff_pol_num = poly(r, dim_array);

    std::vector<std::complex<double>> kern(dim_array + 1);

    for (int kk = 0; kk < dim_array + 1; kk++)
    {

        kern[kk] = exp(-complex_imag * w * double(kk));

    }

    std::complex<double> temp_sum_I;
    std::complex<double> temp_sum_II;

    for (int kk = 0; kk < dim_array + 1; kk++)
    {

        temp_sum_I = 0.0;
        temp_sum_II = 0.0;

        for (int hh = 0; hh < dim_array + 1; hh++)
        {

            temp_sum_I += kern[hh] * den_filt[hh];
            temp_sum_II += kern[hh] * coeff_pol_num[hh];

        }

        num_filt[kk] = real(coeff_pol_num[kk] * temp_sum_I / temp_sum_II);
        save_filt_coeff[0][kk] = num_filt[kk];

    }

}
    //Check the stability of the filter
    bool IIR_Butterworth::check_stability_iir(std::vector<std::vector<double> > coeff_filt)
    {
        bool stability_flag = true;

        //Calculate the roots
        arma::mat roots_den_matrix = arma::zeros(coeff_filt[1].size() - 1, coeff_filt[1].size() - 1);
        for (int kk = 0; kk < coeff_filt[1].size() - 2; kk++)
        {

            roots_den_matrix(kk + 1, kk) = 1;

        }

         for (int kk = 0; kk < coeff_filt[1].size() - 1; kk++)
        {

            roots_den_matrix(0, kk) = -coeff_filt[1][kk + 1];

        }
        
         
        std::vector<double> magnitude_roots_den (coeff_filt[1].size() - 1);
        arma::cx_vec roots_den;
        arma::cx_mat eigvec;

        arma::eig_gen(roots_den,eigvec, roots_den_matrix);
        
        for (int kk = 0; kk < coeff_filt[1].size() - 1; kk++)
        {

            magnitude_roots_den[kk] = abs(roots_den[kk]);

            if (magnitude_roots_den[kk] >= 1)
            {

                stability_flag = false;
                break;
            }

        }

        return stability_flag;

    }
    

