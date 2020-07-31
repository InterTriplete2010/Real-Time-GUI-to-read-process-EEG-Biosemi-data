#pragma once
#include <stdio.h> 
#include <complex.h>

//#define ARMA_DONT_USE_CXX11
#define ARMA_DONT_USE_CXX11_MUTEX
#include <armadillo>

#ifndef IIR_Butterworth_H
#define IIR_Butterworth_H

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

    namespace IIR_B_F
    {

        class IIR_Butterworth

        {


        private:

            //get analog, pre - warped frequencies
            void freq_pre_wrapped(int, double, double);

            //convert to low-pass prototype estimate
            void Wn_f1_Wn_f2(int, double, double);
            
            //Get N - th order Butterworth analog lowpass prototype. It returns the poles of the analog form located on the unit circle in the left-half plane so to have a stable causal system
            void buttap(int);
            
            //Calculate the coefficients of the polynomial (based on Matlab code)
            std::vector<std::complex<double>> poly(std::vector<std::complex<double>>, int);
            
            //Calculate the coefficients of the characteristic polynomial (Bernard Brooks' paper (2016))
            std::vector<std::complex<double>> char_poly(arma::cx_mat, int);
            
            //Calculate the factorial of the given number
            int factorial(int);

            //Method to calculate all the possible combinations. Code taken and slightly modified from: 
            //http://rosettacode.org/wiki/Combinations#C.2B.2B
            std::vector<std::vector<int> > combination_method(int, int, int);
            
            //Transform to state-space
            void zp2ss(int);
            
            //Bilinear transformation to find discrete equivalent
            void bilinear(arma::cx_mat, arma::cx_mat, arma::cx_mat, arma::cx_mat, double, int);
            
            //Transform to zero - pole - gain and polynomial forms
            void zero_pole_gain(arma::cx_mat, int, int, double, double);

        public:

            //Estimate the coeffients of a band-pass filter and return a 2 rows x N coefficients matrix. Row 1 = Numerator; Row 2 = Denumerator
            std::vector<std::vector<double> > lp2bp(double, double, int);
            
            //Estimate the coeffients of a band-stop filter and return a 2 rows x N coefficients matrix. Row 1 = Numerator; Row 2 = Denumerator
            std::vector<std::vector<double> > lp2bs(double, double, int);
            
            //Estimate the coeffients of a low-pass filter and return a 2 rows x N coefficients matrix. Row 1 = Numerator; Row 2 = Denumerator
            std::vector<std::vector<double> > lp2lp(double, int);
            
            //Estimate the coeffients of a high-pass filter and return a 2 rows x N coefficients matrix. Row 1 = Numerator; Row 2 = Denumerator
            std::vector<std::vector<double> > lp2hp(double, int);

            //Check the stability of the filter. Returns "true" is the filter is stable, false if it is unstable 
            bool check_stability_iir(std::vector<std::vector<double> >);

        };

    }

#endif

#ifdef __cplusplus

}

#endif

