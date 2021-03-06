// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

#include <iomanip>
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
using namespace Rcpp;



// EXAMPLE of how to export if desired
////' Reformat the reads for a single individual
////' 
////' @param x Describe parameters here.
////' @export
//// [[Rcpp::export]]

arma::mat rcpp_make_eMatHap_t(
    const Rcpp::List& sampleReads,
    const int nReads,
    const arma::mat& eHaps_t,
    const double maxDifferenceBetweenReads,
    const int Jmax,
    arma::mat& eMatHapPH_t,
    const arma::vec& pRgivenH1,
    const arma::vec& pRgivenH2,
    const bool run_pseudo_haploid = false 
);


//' @export
// [[Rcpp::export]]
Rcpp::List rcpp_make_sampleReads_from_hap(const Rcpp::IntegerVector non_NA_cols, const int reference_phred, const Rcpp::IntegerVector reference_hap) {
    Rcpp::List sampleReads(non_NA_cols.length());
    for(int i = 0; i < non_NA_cols.length(); i++) {
        sampleReads[i]=Rcpp::List::create(0, non_NA_cols[i] - 1, reference_phred * (2 * reference_hap[i] - 1), non_NA_cols[i] - 1);
    }
    return sampleReads;
}




//' @export
// [[Rcpp::export]]
Rcpp::NumericVector increment2N(int yT, int xT, Rcpp::NumericVector y, Rcpp::NumericVector z) {
  Rcpp::NumericVector x(xT+1);
  int t;
  for(t=0; t<=yT-1; t++)
    x[z[t]]=x[z[t]]+y[t];
  return(x);
}



//' @export
// [[Rcpp::export]]
Rcpp::List ram_test(
    const arma::mat& mat1,
    const Rcpp::NumericMatrix& mat2,
    arma::mat mat3,
    Rcpp::NumericMatrix mat4
) {
    double d1 = arma::accu(mat1);
    double d2 = Rcpp::sum(mat2);
    mat3(0, 0) = 3;
    mat4(0, 0) = 2;
    double d3 = arma::accu(mat3);
    double d4 = Rcpp::sum(mat4);
    arma::cube jimmy = arma::zeros(100, 1000, 2);
    return List::create(
                        Rcpp::Named("d1") = d1,
                        Rcpp::Named("d2") = d2,
                        Rcpp::Named("d3") = d3,
                        Rcpp::Named("d4") = d4
                        );
}



//' @export
// [[Rcpp::export]]
Rcpp::List pseudoHaploid_update_model_9(const arma::vec& pRgivenH1, const arma::vec& pRgivenH2, const arma::mat& eMatHap_t1, const arma::mat& eMatHap_t2, const arma::mat& gamma_t1, const arma::mat& gamma_t2, const int K, const arma::ivec& srp) {
    // new stuff
    arma::vec pRgivenH1_new = arma::zeros(pRgivenH1.n_elem);
    arma::vec pRgivenH2_new = arma::zeros(pRgivenH2.n_elem);
    int k, t;
    double d1, d2, x1, x2;
    //
    for(std::size_t i_read=0; i_read < srp.n_elem; i_read++) {
        t = srp(i_read);
        x2 = pRgivenH2(i_read) / \
            (pRgivenH1(i_read) + pRgivenH2(i_read));
        x1 = 1 - x2;
        d1 = x2 * pRgivenH2(i_read);
        d2 = x1 * pRgivenH1(i_read);
        for(k=0; k < K; k++) {
            pRgivenH1_new(i_read) = pRgivenH1_new(i_read) +      \
                gamma_t1(k, t) * \
                (eMatHap_t1(k, i_read) - d1) / x1;
            pRgivenH2_new(i_read) = pRgivenH2_new(i_read) +  \
                gamma_t2(k, t) * \
                (eMatHap_t2(k, i_read) - d2) / x2;
        }
    }
    return List::create(
        Rcpp::Named("pRgivenH1_new") = pRgivenH1_new,
        Rcpp::Named("pRgivenH2_new") = pRgivenH2_new
                        );
}
    








//' @export
// [[Rcpp::export]]
Rcpp::NumericVector get_random_values(int N) {
    Rcpp::NumericVector out = Rcpp::NumericVector(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    int n;
    double rand_uniform;
    for(n=0; n<N; n++){
        rand_uniform = dis(gen);        
        out(n)=rand_uniform;
    }
    return(out);
}




double print_times (double prev, int suppressOutput, std::string past_text, std::string next_text) {
    if( suppressOutput == 0 ) {    
        double cur=clock();
        std::cout << std::setw (40) << past_text;
        printf ("- %.6f cpu sec -", ((double)cur - (double)prev)* 1.0e-6);
        std::cout << next_text << std::endl;
        prev=cur;
    }
    return prev;
}



//' @export
// [[Rcpp::export]]
arma::imat sample_diploid_path(const arma::mat & alphaHat_t, const arma::mat & transMatRate_t_D, const arma::mat & eMat_t, const arma::mat & alphaMat_t, const int T, const int K, const arma::rowvec & c) {
    //
    arma::imat sampled_path_diploid_t(3, T);
    int sampled_state;
    int t;
    int first_k;
    int second_k;
    int KK = K * K;
    double sum = 0;
    double rand_uniform = 0;
    double samp_vector_sum;
    double norm;
    arma::rowvec samp_vector = arma::zeros(1, KK);
    int prev_state;
    int prev_first_k, prev_second_k, k1, kk;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    // somehow this makes dis(gen) give me random 0-1 doubles I think
    //
    // choose initial state
    //
    double check = 0;
    for(kk = 0; kk < KK; kk++)
        check = check + alphaHat_t(kk, T - 1);
    if ((pow(check - 1, 2)) > 1e-4) {
        std::cout << "BAD input assumption" << std::endl;
        return(sampled_path_diploid_t);
    }
    //
    //
    kk = 0;  
    rand_uniform = dis(gen);
    sum = 0;
    while(kk < KK) {
        sum = sum + alphaHat_t(kk, T - 1); // does this sum to 1???
        if (sum > rand_uniform) {
            sampled_state = kk;
            first_k = (sampled_state) % K;
            second_k = (sampled_state - first_k) / K;
            sampled_path_diploid_t(0, T - 1) = first_k; // 0 based
            sampled_path_diploid_t(1, T - 1) = second_k; // 0 based
            sampled_path_diploid_t(2, T - 1) = sampled_state; // 0 based
            kk = KK;
        }
        kk = kk + 1;    
    }
    //
    // cycle
    //
    //for(t = T - 2; t >= 0; t--) {
    for(t = T - 2; t >= 0; t--) {
        prev_first_k = sampled_path_diploid_t(0, t + 1); // 0-based
        prev_second_k = sampled_path_diploid_t(1, t + 1); // 0 based
        prev_state = sampled_path_diploid_t(2, t + 1); // 0 based
        samp_vector.fill(transMatRate_t_D(2, t) * alphaMat_t(prev_first_k, t) * alphaMat_t(prev_second_k, t));
        for(k1=0; k1<=K-1; k1++) {
            // switch on first, keep second
            samp_vector(k1 + K * prev_second_k) = samp_vector(k1 + K * prev_second_k) + transMatRate_t_D(1, t) * alphaMat_t(prev_first_k, t);
            // keep first, switch on second
            samp_vector(prev_first_k + K * k1) = samp_vector(prev_first_k + K * k1) + transMatRate_t_D(1, t) * alphaMat_t(prev_second_k, t);
        }
        samp_vector(prev_state)=samp_vector(prev_state) + transMatRate_t_D(0, t);
        //
        samp_vector_sum = 0;
        for(kk=0; kk<=KK-1; kk++) {
            samp_vector(kk)=samp_vector(kk) * alphaHat_t(kk, t);
            samp_vector_sum=samp_vector_sum + samp_vector(kk);
        }
        norm = alphaHat_t(prev_state, t + 1) / eMat_t(prev_state, t + 1) / c(t + 1);
        if ((pow(samp_vector_sum / norm - 1, 2)) > 1e-4) {
             std::cout << "BAD COUNT on t=" << t << std::endl;
             std::cout << "samp_vector_sum=" << samp_vector_sum << std::endl;
             std::cout << "norm=" << norm << std::endl;
             return(sampled_path_diploid_t);
        }
        // sample state using crazy C++
        kk = 0;  
        rand_uniform = dis(gen);
        sum = 0;
        while(kk < KK) {
            sum = sum + samp_vector(kk) / norm;
            if (sum > rand_uniform) {
                sampled_state = kk;
                kk = KK;
            }
            kk = kk + 1;    
        }
        // convert back here
        first_k = (sampled_state) % K;
        second_k = (sampled_state - first_k) / K;
        sampled_path_diploid_t(0, t) = first_k;
        sampled_path_diploid_t(1, t) = second_k;
        sampled_path_diploid_t(2, t) = sampled_state;
    }
    return(sampled_path_diploid_t);
}



// requires initialization of first column
void run_forward_diploid(
    arma::mat& alphaHat_t,
    arma::rowvec& c,
    const arma::mat& eMat_t,
    const arma::mat& alphaMat_t,    
    const arma::mat& transMatRate_t_D,
    const int& T,
    const int& K
) {
    double alphaConst;
    int kk, k1, k2, K_times_k1;
    arma::vec alphaTemp1 = arma::zeros(K);
    arma::vec alphaTemp2 = arma::zeros(K);
    arma::colvec alphaHat_t_col, alphaMat_t_col;
    double d0, d1, d2;
    for(int t = 1; t < T; t++) {
        // calculate necessary things
        alphaHat_t_col = alphaHat_t.col(t - 1);
        alphaMat_t_col = alphaMat_t.col(t - 1);        
        d0 = transMatRate_t_D(0, t-1);
        d1 = transMatRate_t_D(1, t-1);
        d2 = transMatRate_t_D(2, t-1);
        alphaTemp1.fill(0);
        alphaTemp2.fill(0);
        for(k1 = 0; k1 < K; k1++) {
            K_times_k1 = K * k1;
            for(k2 = 0; k2 < K; k2++) {
                kk = K_times_k1 + k2;
                alphaTemp1(k2) += alphaHat_t_col(kk);
                alphaTemp2(k1) += alphaHat_t_col(kk);
            }
        }
        // now make constant over whole thing
        alphaConst = arma::sum(alphaHat_t_col) * d2;
        alphaTemp1 *= d1;
        alphaTemp2 *= d1;        
        //
        for(k1 = 0; k1 < K; k1++) {
            K_times_k1 = K * k1;            
            for(k2 = 0; k2 < K; k2++) {
                kk = K_times_k1 + k2;
                alphaHat_t(kk, t) = eMat_t(kk, t) *  \
                    (alphaHat_t_col(kk) * d0 +       \
                     (alphaMat_t_col(k1) * alphaTemp1(k2) + \
                      alphaMat_t_col(k2) * alphaTemp2(k1)) + \
                     alphaMat_t_col(k1) * alphaMat_t_col(k2) * alphaConst);
            }
        }
        // do scaling now
        c(t) = 1 / sum(alphaHat_t.col(t));
        alphaHat_t.col(t) *= c(t);
    }
    return;
}


// requires initialization of first column
void run_backward_diploid(
    arma::mat& betaHat_t,
    arma::rowvec& c,
    const arma::mat& eMat_t,
    const arma::mat& alphaMat_t,    
    const arma::mat& transMatRate_t_D,
    const int& T,
    const int& K
) {
    int t, k1, k2, kk, K_times_k1;
    arma::vec betaTemp1 = arma::zeros(K);
    arma::vec betaTemp2 = arma::zeros(K);
    double betaConst, d, x, d0;
    arma::colvec alphaMat_t_col, betaHat_mult_eMat_t_col;
    for(t = T-2; t>=0; --t) {
        betaTemp1.fill(0);
        betaTemp2.fill(0);
        betaConst=0;
        alphaMat_t_col = alphaMat_t.col(t);
        betaHat_mult_eMat_t_col = betaHat_t.col(t + 1) % eMat_t.col(t + 1); // element-wise multiplication
        for(k1 = 0; k1 < K; k1++) {
            K_times_k1 = K * k1;
            for(k2 = 0; k2 < K; k2++) {
                kk = K_times_k1 + k2;
                d = betaHat_mult_eMat_t_col(kk);
                betaTemp1(k1) += d * alphaMat_t_col(k2);
                betaTemp2(k2) += d * alphaMat_t_col(k1);
                betaConst += d * alphaMat_t_col(k1) * alphaMat_t_col(k2);
            }
        }
        // add transMatRate to constants
        d = transMatRate_t_D(1,t);
        betaTemp1 *= d;
        betaTemp2 *= d;
        betaConst *= transMatRate_t_D(2, t);
        d0 = transMatRate_t_D(0, t);
        // final calculation
        for(k1 = 0; k1 < K; k1++) {
            x = betaTemp1(k1) + betaConst;
            K_times_k1 = K * k1;            
            for(k2 = 0; k2 < K; k2++) {
                kk = K_times_k1 + k2;                
                betaHat_t(kk, t) = betaHat_mult_eMat_t_col(kk) * d0 + betaTemp2(k2) + x;
            }
        }
        // apply scaling
        betaHat_t.col(t) *= c(t);
    }
    return;
}


// requires initialization of first column
void run_forward_haploid(
    arma::mat& alphaHat_t,
    arma::rowvec& c,
    const arma::mat& eMatHapSNP_t,
    const arma::mat& alphaMat_t,    
    const arma::mat& transMatRate_t_H,
    const int& T,
    const int& K
) {
    double alphaConst;
    //
    for(int t = 1; t < T; t++) {
        alphaConst = transMatRate_t_H(1, t-1) * arma::sum(alphaHat_t.col(t - 1));
        //
        alphaHat_t.col(t) = eMatHapSNP_t.col(t) % ( \
            transMatRate_t_H(0, t - 1) * alphaHat_t.col(t - 1) + \
            alphaConst * alphaMat_t.col(t - 1) );
        //
        c(t) = 1 / arma::sum(alphaHat_t.col(t));
        alphaHat_t.col(t) *= c(t);
    }
    return ;
}



void run_backward_haploid(
    arma::mat& betaHat_t,
    arma::rowvec& c,
    const arma::mat& eMatHapSNP_t,
    const arma::mat& alphaMat_t,    
    const arma::mat& transMatRate_t_H,
    const int& T,
    const int& K
) {
    double x;
    arma::colvec e_times_b;
    for(int t = T-2; t >= 0; --t) {
        //x = 0;
        e_times_b = eMatHapSNP_t.col(t+1) % betaHat_t.col(t+1);
        x = transMatRate_t_H(1, t) * sum(alphaMat_t.col(t) % e_times_b);
        betaHat_t.col(t) = c(t) * (x + transMatRate_t_H(0, t) * e_times_b);
    }
    return;
}


//' @export
// [[Rcpp::export]]
arma::mat rcpp_make_and_bound_eMat_t(
    const arma::mat& eMatHap_t,
    const Rcpp::List& sampleReads,
    const int& nReads,
    const int& K,
    const int& T,
    const double& maxEmissionMatrixDifference,
    const int run_fb_grid_offset = 0
) {
    int readSNP;
    double x, rescale;
    const int KK = K * K;
    arma::mat eMat_t = arma::ones(KK, T);
    arma::colvec eMatHap_t_col;
    int iGrid, k3, k, k1, k2;
    for(int iRead=0; iRead < nReads; iRead++) {
        Rcpp::List readData = as<Rcpp::List>(sampleReads[iRead]);
        readSNP = as<int>(readData[1]) - run_fb_grid_offset; // leading SNP from read
        eMatHap_t_col = 0.5 * eMatHap_t.col(iRead);
        for(k1 = 0; k1 < K; k1++) {
            x = eMatHap_t_col(k1);
            k3 = K * k1;
            for(k2 = 0; k2 < K; k2++) {
                eMat_t(k3 + k2, readSNP) *= (x + eMatHap_t_col(k2));                
            }
        }  // end of SNP in read
    }
    //
    // cap eMat, ie P(reads | k1,k2) to be within maxDifferenceBetweenReads^2
    //
    double one_over_maxEmissionMatrixDifference = 1 / maxEmissionMatrixDifference;
    // loop over eMat_t
    for(iGrid = 0; iGrid < T; iGrid++) {
        if (eMat_t(0, iGrid) < 1) {
            // first, get maximum
            x = eMat_t.col(iGrid).max();
            // x is the maximum now. re-scale to x
            rescale = 1 / x;        
            for(k=0; k < KK; k++) {
                eMat_t(k, iGrid) *= rescale;
                if(eMat_t(k, iGrid)<(one_over_maxEmissionMatrixDifference))
                    eMat_t(k, iGrid) = one_over_maxEmissionMatrixDifference;
            }
        }
    } // end of loop on t
    return eMat_t;
}


//' @export
// [[Rcpp::export]]
Rcpp::List rcpp_make_fb_snp_offsets(
    const arma::mat& alphaHat_t,
    const arma::mat& betaHat_t,
    const arma::mat& blocks_for_output
) {
    int s, e;
    arma::mat alphaHatBlocks_t = arma::zeros(alphaHat_t.n_rows, blocks_for_output.n_rows);
    arma::mat betaHatBlocks_t = arma::zeros(betaHat_t.n_rows, blocks_for_output.n_rows);    
    for(int i_output=0; i_output < blocks_for_output.n_rows; i_output++) {
        s = blocks_for_output(i_output, 2); // these are 0-based. these are the grid entries
        e = blocks_for_output(i_output, 3);
        alphaHatBlocks_t.col(i_output) = alphaHat_t.col(s);
        betaHatBlocks_t.col(i_output) = betaHat_t.col(e);
    }
    return(wrap(Rcpp::List::create(
                                   Rcpp::Named("alphaHatBlocks_t") = alphaHatBlocks_t,
                                   Rcpp::Named("betaHatBlocks_t") = betaHatBlocks_t
                                   )));
}


//' @export
// [[Rcpp::export]]
void rcpp_make_diploid_jUpdate(
    arma::mat& jUpdate_t,
    const int K,
    const int T,
    const arma::mat& alphaHat_t,
    const arma::mat& betaHat_t,    
    const arma::mat& transMatRate_t_D,
    const arma::mat& alphaMat_t,
    const arma::mat& eMat_t
) {
    int t, k1, k2, kk, K_times_k1;
    arma::vec alphaTemp1 = arma::zeros(K);
    arma::vec alphaTemp2 = arma::zeros(K);
    double tmr1, tmr2, d;
    arma::colvec alphaMat_t_col_times_tmr2, betaHat_times_eMat;
    //
    for(t=0; t<=T-2; t++) {
        alphaTemp1.fill(0);
        for(k1 = 0; k1 < K; k1++) {
            K_times_k1 = K * k1;
            for(k2 = 0; k2 < K; k2++) {
                alphaTemp1(k2) += alphaHat_t(K_times_k1 + k2, t);
            }
        }
        //
        // now do proper calculation
        //
        tmr1 = transMatRate_t_D(1, t);
        tmr2 = transMatRate_t_D(2, t);
        alphaMat_t_col_times_tmr2 = alphaMat_t.col(t) * tmr2;
        alphaTemp1 *= tmr1;
        alphaTemp1 += alphaMat_t_col_times_tmr2;
        betaHat_times_eMat = betaHat_t.col(t + 1) % eMat_t.col(t + 1);
        for(k1 = 0; k1 < K; k1++) {
            K_times_k1 = K * k1;
            d = 0;
            for(k2 = 0; k2 < K; k2++) {
                kk = K_times_k1 + k2;
                d += alphaTemp1(k2) * betaHat_times_eMat(kk);
                // jUpdate_t(k1, t) += alphaTemp1(k2) * betaHat_t(kk, t + 1) * eMat_t(kk, t + 1);
            }
            d *= 2 * alphaMat_t(k1, t);
            jUpdate_t(k1, t) += d;
        }   // end of loop on k
    } // end of loop on t
    return;
};



//' @export
// [[Rcpp::export]]
arma::mat rcpp_calculate_fbd_dosage(
    const arma::mat& eHapsCurrent_t,
    const arma::mat& gamma_t,
    const Rcpp::IntegerVector& grid,
    const int snp_start_1_based,
    const int snp_end_1_based,
    const int grid_offset = 0
) {
    // basically, copy and paste, either using grid or not
    const int nSNPs = snp_end_1_based - snp_start_1_based + 1;
    const int K = eHapsCurrent_t.n_rows;
    arma::mat genProbs_t = arma::zeros(3, nSNPs);
    //arma::vec dosage = arma::zeros(nSNPs);
    // new
    int iSNP, t, k1, k2, kk, K_times_k1, cur_grid;
    double a, b, one_minus_b, g, one_minus_a, g0, g1, g2;
    arma::colvec eHapsCurrent_t_col, gamma_t_col, one_minus_eHapsCurrent_t_col;
    int prev_grid = -1;
    // i_t is index from 0 to nSNPs + 1, controls where things go
    // t is the index in the whole set of SNPs
    // tt is the index in the grid
    /// grid_offset refers to in what grids we are running
    for(iSNP = 0; iSNP < nSNPs; iSNP++) {
        g0 = 0;
        g1 = 0;
        g2 = 0;
        t = iSNP + snp_start_1_based - 1;
        cur_grid = grid(t) - grid_offset;
        if (cur_grid > prev_grid) {
            gamma_t_col = gamma_t.col(cur_grid);
            prev_grid = cur_grid;
        }
        eHapsCurrent_t_col = eHapsCurrent_t.col(t);
        one_minus_eHapsCurrent_t_col = 1 - eHapsCurrent_t_col;
        for(k1 = 0; k1 <K; k1++) {
            a = eHapsCurrent_t_col(k1);
            one_minus_a = one_minus_eHapsCurrent_t_col(k1);
            K_times_k1 = K * k1;
            for(k2 = 0; k2 < K; k2++) {
                kk = K_times_k1 + k2; // does not matter which way I do this
                g = gamma_t_col(kk);
                b = eHapsCurrent_t_col(k2);
                one_minus_b = one_minus_eHapsCurrent_t_col(k2);
                g0 += g * one_minus_a * one_minus_b;
                g1 += g * (a * one_minus_b + one_minus_a * b);
                g2 += g * a * b;
            }
        }
        genProbs_t(0, iSNP) = g0;
        genProbs_t(1, iSNP) = g1;
        genProbs_t(2, iSNP) = g2;        
        //for(j=1;j<=2;j++) {
        //    dosage(i_t) = dosage(i_t) + j * genProbs_t(j, i_t);
        //}
    }
    return(genProbs_t);
}


void calculate_diploid_gammaUpdate(
    arma::cube& gammaUpdate_t,
    const Rcpp::List& sampleReads,
    const int nReads,
    const arma::mat& gamma_t,
    const arma::mat& eHapsCurrent_t,
    const arma::mat& eMatHap_t 
) {
    //
    const int K = eHapsCurrent_t.n_rows;
    //
    arma::colvec eMatHap_t_col;
    arma::colvec eHapsCurrent_t_col, gamma_t_col;
    arma::ivec bqU, pRU;    
    int J, cr, j, iRead, k1, k2, t, K_times_k1, kk;
    int cr_prev = -1;
    double eps, pA, pR, d1, d2, d3, a, b, e, val1, val2;
    //
    for(iRead = 0; iRead < nReads; iRead++) {
        // recal that below is what is used to set each element of sampleRead
        // sampleReads.push_back(Rcpp::List::create(nU,d,phiU,pRU));
        Rcpp::List readData = as<Rcpp::List>(sampleReads[iRead]);
        J = readData[0]; // number of SNPs on read
        cr = readData[1]; // central SNP or grid point
        bqU = as<arma::ivec>(readData[2]); // bq for each SNP
        pRU = as<arma::ivec>(readData[3]); // position of each SNP from 0 to T-1
        // loop over every SNP in the read
        if (cr > cr_prev) {
            gamma_t_col = gamma_t.col(cr);
            cr_prev = cr;
        }
        for(j = 0; j <= J; j++) {
            t=pRU(j); // position of this SNP in full T sized matrix
            //
            // first haplotype (ie (k,k1))
            //
            // less than 0 - reference base more likely
            if(bqU(j)<0) {
                eps = pow(10,(double(bqU(j))/10));
                pR=1-eps;
                pA=eps/3;
            }
            if(bqU(j)>0) {
                eps = pow(10,(-double(bqU(j))/10));
                pR=eps/3;
                pA=1-eps;
            }
            //
            // RECAL  eMatHap(iRead,k) = eMatHap(iRead,k) * ( eHaps(pRU(j),k) * pA + (1-eHaps(pRU(j),k)) * pR);
            // RECAL  eMat(readSNP,k1+K*k2) = eMat(readSNP,k1+K*k2) * (0.5 * eMatHap(iRead,k1) + 0.5 * eMatHap(iRead,k2));
            //
            eMatHap_t_col = eMatHap_t.col(iRead);
            eHapsCurrent_t_col = eHapsCurrent_t.col(t);
            for(k1 = 0; k1 < K; k1++) {
                d1 = pA * eHapsCurrent_t_col(k1);
                d2 = pR * (1 - eHapsCurrent_t_col(k1));
                d3 = d1 / (d1 + d2); // this is all I need
                a = eMatHap_t_col(k1);
                K_times_k1 = K * k1;
                val1 = 0;
                val2 = 0;
                for(k2 = 0; k2 < K; k2++) {
                    b = eMatHap_t_col(k2);
                    kk = K_times_k1 + k2;                    
                    e = gamma_t_col(kk) * ( a / (a + b));
                    val1 += e * d3;
                    val2 += e;
                }
                gammaUpdate_t(k1, t, 0) += 2 * val1; // this way I save the *2 multiplication until the end
                gammaUpdate_t(k1, t, 1) += 2 * val2;
            } // end of loop onk
        } // end of loop on SNP within read
    }
    return;
}


//' @export
// [[Rcpp::export]]
arma::mat make_gammaEK_t_from_gammaK_t(
    const arma::mat& gammaK_t,
    const int K,
    const Rcpp::IntegerVector& grid,
    const int snp_start_1_based,
    const int snp_end_1_based,
    const int grid_offset = 0
) {
    const int nSNPs = snp_end_1_based - snp_start_1_based + 1;
    arma::mat gammaEK_t = arma::zeros(K, nSNPs);
    int t;
    int prev_grid = -1;
    int iSNP;
    int cur_grid;
    arma::colvec gamma_t_col;
    for(iSNP = 0; iSNP < nSNPs; iSNP++) {
        t = iSNP + snp_start_1_based - 1;
        cur_grid = grid(t) - grid_offset;
        if (cur_grid > prev_grid) {
            gamma_t_col = gammaK_t.col(cur_grid);
            prev_grid = cur_grid;
        }
        gammaEK_t.col(iSNP) = gamma_t_col;
    }
    return(gammaEK_t);
}

//' @export
// [[Rcpp::export]]
arma::mat collapse_diploid_gamma(
    const arma::mat& gamma_t,
    const int T,
    const int K
) {
    arma::mat gammaK_t = arma::zeros(K, T);
    int K_times_k1;
    int t, k1, k2;
    double d;
    for(t = 0; t < T; t++) {
        for(k1 = 0; k1 < K; k1++) {
            d = 0;
            K_times_k1 = K * k1;
            for(k2 = 0; k2 < K; k2++) {
                d += gamma_t(K_times_k1 + k2, t);
            }
            gammaK_t(k1, t) = d;
        }
    }
    return(gammaK_t);
}



//' @export
// [[Rcpp::export]]
Rcpp::List forwardBackwardDiploid(
    const Rcpp::List& sampleReads,
    const int nReads,
    const arma::vec& pi,
    const arma::mat& transMatRate_t_D,
    const arma::mat& alphaMat_t,
    const arma::mat& eHaps_t,
    arma::mat& alphaHat_t,
    arma::mat& betaHat_t,
    const double maxDifferenceBetweenReads,
    const double maxEmissionMatrixDifference,
    const int Jmax,
    const int suppressOutput,
    const arma::mat& blocks_for_output,
    arma::cube& gammaUpdate_t, // see update_in_place
    arma::mat& jUpdate_t,
    arma::mat& hapSum_t,
    Rcpp::NumericVector& priorSum,
    const bool generate_fb_snp_offsets = false,
    const Rcpp::NumericVector alphaStart = 0,
    const Rcpp::NumericVector betaEnd = 0,
    const bool return_a_sampled_path = false,
    const bool run_fb_subset = false,
    const int run_fb_grid_offset = 0, // this is 0-based
    const bool return_genProbs = false, // the below are needed if we want genProbs. dosage trivial
    const int snp_start_1_based = -1,
    const int snp_end_1_based = -1,
    const Rcpp::IntegerVector grid = 0, // end of things needed for genProbs and dosage
    const bool return_gamma = false, // full gamma, K * K rows
    const bool return_extra = false, // whether to return stuff useful for debugging
    const bool update_in_place = false, // update directly into output variables
    const bool pass_in_alphaBeta = false, // whether to pass in pre-made alphaHat, betaHat
    const bool output_haplotype_dosages = false // whether to output state probabilities
) {
  double prev=clock();
  std::string prev_section="Null";
  std::string next_section="Initialize variables";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  // constants
  //
  const int T = alphaMat_t.n_cols + 1;  // what we iterate over / grid
  const int nSNPs = eHaps_t.n_cols; // traditional K for haplotypes  
  const int K = eHaps_t.n_rows; // traditional K for haplotypes
  const int KK = K*K; // KK is number of states / traditional K for HMMs
  //
  // new variables
  //
  // variables working on nReads
  if (!pass_in_alphaBeta) {
      alphaHat_t = arma::zeros(KK, T);  
      betaHat_t = arma::zeros(KK, T);
  }
  arma::rowvec c = arma::zeros(1, T);
  // variables for faster forward backward calculation  double alphaConst, betaConst;
  arma::vec alphaTemp1 = arma::zeros(K);
  arma::vec alphaTemp2 = arma::zeros(K);
  // variables working on full space
  int kk, t, k1, k2;
  double d;
  Rcpp::List alphaBetaBlocks;
  Rcpp::List to_return;
  arma::mat genProbs_t;  
  //
  //
  // eMat - make complete
  //
  //
  next_section="Initialize eMatHap";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  // dummy variables
  arma::mat gammaK_t;
  arma::mat gammaEK_t;
  arma::mat eMatHapPH_t;
  arma::vec pRgivenH1;
  arma::vec pRgivenH2;
  arma::mat eMatHap_t = rcpp_make_eMatHap_t(
      sampleReads,
      nReads,
      eHaps_t,
      maxDifferenceBetweenReads,
      Jmax,
      eMatHapPH_t,
      pRgivenH1,
      pRgivenH2
  );
  //
  // once we have all the eMatHaps, ie probabilities from reads, make eMat from this
  //
  next_section="Initialize and bound eMat";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  arma::mat eMat_t = rcpp_make_and_bound_eMat_t(eMatHap_t, sampleReads, nReads, K, T, maxEmissionMatrixDifference, run_fb_grid_offset);
  //
  // forward recursion
  //
  next_section="Forward recursion";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  if (run_fb_subset == false) {
      for(k1=0; k1<=K-1; k1++)
          for(k2=0; k2<=K-1; k2++)
              alphaHat_t(k1+K*k2,0) = pi(k1) * pi(k2) * eMat_t(k1+K*k2,0);
  } else {
      for(kk=0; kk<=KK-1; kk++) {
          alphaHat_t(kk, 0) = alphaStart(kk);
      }
  }
  c(0) = 1 / sum(alphaHat_t.col(0));
  alphaHat_t.col(0) = alphaHat_t.col(0) * c(0);
  run_forward_diploid(alphaHat_t, c, eMat_t, alphaMat_t, transMatRate_t_D, T, K);
  //
  //
  //
  // backward recursion
  //
  //
  next_section="Backward recursion";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  if (run_fb_subset == false) {  
      betaHat_t.col(T-1).fill(c(T-1));
  } else {
      for(kk=0; kk<=KK-1; kk++) {
          betaHat_t(kk, T-1) = betaEnd(kk);
      }
  }
  run_backward_diploid(betaHat_t, c, eMat_t, alphaMat_t, transMatRate_t_D, T, K);
  //
  //
  // make gamma
  //
  //
  next_section="Make gamma";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  arma::mat gamma_t = alphaHat_t % betaHat_t;
  double g_temp = 0;
  for(t = 0; t < T; t++) {
      g_temp = 1 / c(t);
      gamma_t.col(t) *= g_temp;
  }
  //
  // (optional) calculate genProbs and dosage
  //
  if (return_genProbs) {
      next_section="Make genProbs_t";
      prev=print_times(prev, suppressOutput, prev_section, next_section);
      prev_section=next_section;
      genProbs_t = rcpp_calculate_fbd_dosage(
          eHaps_t,
          gamma_t,
          grid,
          snp_start_1_based,
          snp_end_1_based,
          run_fb_grid_offset
      );
      to_return.push_back(genProbs_t, "genProbs_t");      
  }
  // make outputs here
  if (generate_fb_snp_offsets) {
      alphaBetaBlocks = rcpp_make_fb_snp_offsets(
          alphaHat_t,
          betaHat_t,
          blocks_for_output
      );
  }
  //
  // make collapsed gamma here
  //
  // skip if run_fb_dosage and we don't want output haplotype dosages
  if (!(run_fb_subset & !output_haplotype_dosages)) {
      next_section="Make collapsed gamma";
      prev=print_times(prev, suppressOutput, prev_section, next_section);
      prev_section=next_section;
      gammaK_t = collapse_diploid_gamma(gamma_t, T, K);
      if (output_haplotype_dosages) {
          gammaEK_t = make_gammaEK_t_from_gammaK_t(
              gammaK_t, K, grid,
              snp_start_1_based, snp_end_1_based, run_fb_grid_offset
          );
      }
  }
  //
  // optional, end early
  //
  if (run_fb_subset) {
      if (output_haplotype_dosages) {
          to_return.push_back(gammaEK_t, "gammaEK_t");
      }
      return(to_return);
  }
  //
  //
  //
  if (update_in_place) {  
      hapSum_t += gammaK_t;
      for(int k=0; k < K; k++) {
          priorSum(k) = priorSum(k) + gammaK_t(k, 0);
      }
  }
  //
  //
  // do gamma update here - save large matrix, just put in necessary values
  //
  //
  next_section="Gamma update";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  if (!update_in_place) {    
      gammaUpdate_t = arma::zeros(K, nSNPs, 2);      
  }
  calculate_diploid_gammaUpdate(
      gammaUpdate_t,
      sampleReads,
      nReads,
      gamma_t,
      eHaps_t,
      eMatHap_t 
  );
  //
  //
  // make jUpdate
  //
  //
  next_section="Make xi-like calculations";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  if (!update_in_place) {    
      jUpdate_t = arma::zeros(K, T - 1);
  }
  rcpp_make_diploid_jUpdate(
      jUpdate_t,
      K,
      T,
      alphaHat_t,
      betaHat_t,      
      transMatRate_t_D,
      alphaMat_t,
      eMat_t
  );
  //
  // optional, sample a path
  //
  arma::imat sampled_path_diploid_t;
  if (return_a_sampled_path) {
      sampled_path_diploid_t = sample_diploid_path(alphaHat_t, transMatRate_t_D, eMat_t, alphaMat_t, T, K, c);
  }
  //
  //
  // done - return necessary information
  //
  //
  next_section="Done";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  to_return.push_back(gammaK_t, "gammaK_t");
  if (!update_in_place) {    
      to_return.push_back(gammaUpdate_t, "gammaUpdate_t");
      to_return.push_back(jUpdate_t, "jUpdate_t");      
  }
  if (generate_fb_snp_offsets) {
      to_return.push_back(alphaBetaBlocks, "alphaBetaBlocks");
  }
  // rest largely for debugging
  if (return_extra) {
      to_return.push_back(alphaHat_t, "alphaHat_t");
      to_return.push_back(betaHat_t, "betaHat_t");
      to_return.push_back(eMat_t, "eMat_t");
      to_return.push_back(eMatHap_t, "eMatHap_t");
      to_return.push_back(c, "c");
  }
  if (return_gamma) { 
      to_return.push_back(gamma_t, "gamma_t");
  }
  // deprecated?
  if (return_a_sampled_path) {
      to_return.push_back(sampled_path_diploid_t, "sampled_path_diploid_t");
  }
  if (output_haplotype_dosages) {
      to_return.push_back(gammaEK_t, "gammaEK_t");
  }
  return(to_return);
}






//' @export
// [[Rcpp::export]]
void make_haploid_gammaUpdate_t(
    arma::cube& gammaUpdate_t,
    const Rcpp::List& sampleReads,
    const int nReads,
    const arma::mat& gamma_t,
    const arma::mat& eHapsCurrent_t,
    const arma::mat& eMatHap_t,    
    const arma::mat& eMatHapOri_t,
    const arma::vec& pRgivenH1,
    const arma::vec& pRgivenH2,
    const bool run_pseudo_haploid = false
) {
    //
    //const int nSNPs = eHapsCurrent_t.n_cols;
    const int K = eHapsCurrent_t.n_rows;
    //
    //arma::cube gammaUpdate_t = arma::zeros(K, nSNPs, 2);
    int iRead, J, cr, t, j, k;
    Rcpp::List readData;
    arma::ivec bqU, pRU;
    arma::colvec gamma_t_col;
    double d3, eps, a1, a2, y, d, d1, d2, b;
    double pR = -1;
    double pA = -1;
    int cr_prev = -1;
    //
    for(iRead = 0; iRead < nReads; iRead++) {
        readData = as<Rcpp::List>(sampleReads[iRead]);
        J = as<int>(readData[0]); // number of SNPs on read
        cr = as<int>(readData[1]); // central SNP in read
        bqU = as<arma::ivec>(readData[2]); // bq for each SNP
        pRU = as<arma::ivec>(readData[3]); // position of each SNP from 0 to T-1
        if (run_pseudo_haploid == true) {      
          d3 = pRgivenH1(iRead) / (pRgivenH1(iRead) + pRgivenH2(iRead));
        }
        if (cr > cr_prev) {
            // do not always need to update
            gamma_t_col = gamma_t.col(cr);
        }
        for(j=0; j<=J; j++) {
            t=pRU(j);
            if(bqU(j)<0) {
                eps = pow(10,(double(bqU(j))/10));
                pR=1-eps;
                pA=eps/3;
            }
            if(bqU(j)>0) {
                eps = pow(10,(-double(bqU(j))/10));
                pR=eps/3;
                pA=1-eps;
            }
            //
            if (run_pseudo_haploid == true) {
                for(k=0; k<=K-1;k++) {
                    a1 = pA * eHapsCurrent_t(k,t);
                    a2 = pR * (1-eHapsCurrent_t(k,t)); 
                    y = a1 + a2;
                    b = eMatHapOri_t(k,iRead) * d3 / y;
                    d1 = a1 * b;
                    d2 = a2 * b;
                    d = gamma_t_col(k) / eMatHap_t(k,iRead);
                    gammaUpdate_t(k, t, 0) += d * d1;
                    gammaUpdate_t(k, t, 1) += d * (d1 + d2);
                }
            } else {
                for(k = 0; k < K; k++) {
                    a1 = pA * eHapsCurrent_t(k, t);
                    a2 = pR * (1 - eHapsCurrent_t(k, t));
                    y = a1 + a2;
                    d = gamma_t_col(k) / y;
                    gammaUpdate_t(k, t, 0) += a1 * d;
                    gammaUpdate_t(k, t, 1) += y * d;
                }
            }
        } // end of SNP in read 
    } // end of read
    return;
}


//' @export
// [[Rcpp::export]]
Rcpp::List forwardBackwardHaploid(
    const Rcpp::List& sampleReads,
    const int nReads,
    const arma::vec pi,
    const arma::mat& transMatRate_t_H,
    const arma::mat& alphaMat_t,
    const arma::mat& eHaps_t,
    arma::mat& alphaHat_t,
    arma::mat& betaHat_t,
    const double maxDifferenceBetweenReads,
    const double maxEmissionMatrixDifference,
    const int Jmax,
    const int suppressOutput,
    const int model,
    arma::cube& gammaUpdate_t, // see use_supplied_gammaUpdate_t
    arma::mat& jUpdate_t,
    arma::mat& hapSum_t,
    Rcpp::NumericVector& priorSum,
    const arma::vec& pRgivenH1,
    const arma::vec& pRgivenH2,
    const bool run_pseudo_haploid,
    const arma::mat& blocks_for_output,
    const bool generate_fb_snp_offsets = false,
    const Rcpp::NumericVector alphaStart = 0,
    const Rcpp::NumericVector betaEnd = 0,
    const bool run_fb_subset = false,
    const int run_fb_grid_offset = 0, // this is 0-based
    const bool return_extra = false,
    const bool update_in_place = false,
    const bool pass_in_alphaBeta = false, // whether to pass in pre-made alphaHat, betaHat
    const bool output_haplotype_dosages = false, // whether to output state probabilities
    const int snp_start_1_based = -1,
    const int snp_end_1_based = -1,
    const Rcpp::IntegerVector grid = 0
) {
  double prev=clock();
  std::string prev_section="Null";
  std::string next_section="Initialize variables";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  // constants
  //
  //
  const int T = alphaMat_t.n_cols + 1;  // what we iterate over / grid
  const int nGrids = T; // will make change eventually
  const int K = eHaps_t.n_rows; // traditional K for haplotypes
  //
  // new variables
  //
  // variables working on nReads
  if (!pass_in_alphaBeta) {
      alphaHat_t = arma::zeros(K, T);
      betaHat_t = arma::zeros(K, T);
  }
  arma::rowvec c = arma::zeros(1,T);  
  // eMatHapSNP works on the SNPs themselves
  arma::mat gamma_t = arma::zeros(K, T);  
  arma::mat eMatHapSNP_t = arma::ones(K, T);
  // variables for transition matrix and initialization
  // int variables and such
  int t, k1, iRead, readSNP, k;
  double d = 1;
  double rescale, x;
  Rcpp::List alphaBetaBlocks;
  Rcpp::List to_return;  
  //
  //
  //
  // eMatHap
  //
  //
  //
  next_section="Make eMatHap";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  // define eMatHap to work on the number of reads
  arma::mat eMatHapOri_t;
  if (run_pseudo_haploid) {
      eMatHapOri_t = arma::zeros(K, nReads);
  }
  arma::mat eMatHap_t = rcpp_make_eMatHap_t(
      sampleReads,
      nReads,
      eHaps_t,
      maxDifferenceBetweenReads,
      Jmax,
      eMatHapOri_t,
      pRgivenH1,
      pRgivenH2,
      run_pseudo_haploid
  );
  //
  //
  // once we have eMatHap, ie probabilities from reads, make eMatHapSNPsfrom this
  //
  //
  next_section="Make eMat";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  for(iRead=0; iRead<=nReads-1; iRead++) {
    Rcpp::List readData = as<Rcpp::List>(sampleReads[iRead]);
    readSNP = as<int>(readData[1]) - run_fb_grid_offset; // leading SNP from read
    for(k = 0; k < K; k++) {
        eMatHapSNP_t(k, readSNP) *= eMatHap_t(k, iRead);
    }
  }
  //
  //
  // afterwards - cap per-SNP by difference squared 
  //
  //
  // now - afterward - cap eMatHapSNP
  for(t=0; t<=T-1; t++) {
      // if eMatHapSNP(t, 0) != 0, proceed
      if (eMatHapSNP_t(0, t) > 0) {
          x=0;
          for(k=0; k<=K-1; k++)
              if(eMatHapSNP_t(k, t)>x)
                  x=eMatHapSNP_t(k, t);
          // x is the maximum now
          rescale = 1 / x;        
          x=x/d;
          for(k=0; k<=K-1; k++) {
              eMatHapSNP_t(k, t) = eMatHapSNP_t(k, t) * rescale;
              if(eMatHapSNP_t(k, t) < (1 / maxEmissionMatrixDifference)) {
                  eMatHapSNP_t(k, t)=1 / maxEmissionMatrixDifference;
              }
          }
      }
  }
  //
  //
  // forward recursion
  //
  //
  next_section="Forward recursion";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  if (run_fb_subset == false) {
      for(k1=0; k1<=K-1; k1++)
          alphaHat_t(k1,0) = pi(k1) * eMatHapSNP_t(k1,0);
  } else {
      for(k=0; k<=K-1; k++) {
          alphaHat_t(k, 0) = alphaStart(k);
      }
  }
  c(0) = 1 / sum(alphaHat_t.col(0));
  alphaHat_t.col(0) = alphaHat_t.col(0) * c(0);
  run_forward_haploid(
      alphaHat_t,
      c,
      eMatHapSNP_t,
      alphaMat_t,
      transMatRate_t_H,
      T,
      K
  );
  //
  //
  // backward recursion
  //
  //
  next_section="Backward recursion";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  if (run_fb_subset == false) {    
      betaHat_t.col(T-1).fill(c(T-1));
  } else {
      for(k=0; k<=K-1; k++) {
          betaHat_t(k, T-1) = betaEnd(k);
      }
  }
  run_backward_haploid(
      betaHat_t,
      c,
      eMatHapSNP_t,
      alphaMat_t,
      transMatRate_t_H,
      T,
      K
  );
  //
  //
  // make gamma
  //
  //
  next_section="Make gamma";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  gamma_t = alphaHat_t % betaHat_t;
  // normalize as well
  double g_temp = 0;
  for(t = 0; t < T; t++) {
      g_temp = 1 / c(t);
      gamma_t.col(t) *= g_temp;
  }
  //
  // optional early return, for final iteration
  //
  if (output_haplotype_dosages) {
      // calculate expanded version? for output?
      arma::mat gammaEK_t = make_gammaEK_t_from_gammaK_t(
          gamma_t, K, grid,
          snp_start_1_based, snp_end_1_based, run_fb_grid_offset
      );
      to_return.push_back(gammaEK_t, "gammaEK_t");      
  }
  //
  to_return.push_back(gamma_t, "gamma_t");
  //
  if (run_fb_subset == true) {
      return(to_return);
  }
  // make outputs here
  if (generate_fb_snp_offsets == true) {
      alphaBetaBlocks = rcpp_make_fb_snp_offsets(
          alphaHat_t,
          betaHat_t,
          blocks_for_output
      );
  }
  if (update_in_place) {  
      hapSum_t += gamma_t; // same as gammaK_t
      for(k=0; k < K; k++) {
          priorSum(k) = priorSum(k) + gamma_t(k, 0);
      }
  }
  //
  //
  //
  // hap probs are gamma!
  //
  //
  next_section="Gamma update";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  if (!update_in_place) {        
      const int nSNPs = eHaps_t.n_cols;
      gammaUpdate_t = arma::zeros(K, nSNPs, 2);
  }
  make_haploid_gammaUpdate_t(  
      gammaUpdate_t,
      sampleReads,
      nReads,
      gamma_t,
      eHaps_t,
      eMatHap_t,      
      eMatHapOri_t,
      pRgivenH1,
      pRgivenH2,
      run_pseudo_haploid
  );
  //
  //
  // make jUpdate
  //
  //
  next_section="Make jUpdate";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  if (!update_in_place) {
      jUpdate_t = arma::zeros(K, nGrids - 1);
  }
  for(t = 0; t < T - 1; t++) {
      jUpdate_t.col(t) += transMatRate_t_H(1, t) * (alphaMat_t.col(t) % betaHat_t.col(t + 1) % eMatHapSNP_t.col(t + 1));
  }
  //
  //
  //
  next_section="Done";
  prev=print_times(prev, suppressOutput, prev_section, next_section);
  prev_section=next_section;
  //
  to_return.push_back(eMatHap_t, "eMatHap_t");
  to_return.push_back(eMatHapOri_t, "eMatHapOri_t");
  if (!update_in_place) {
      to_return.push_back(gammaUpdate_t, "gammaUpdate_t");
      to_return.push_back(jUpdate_t, "jUpdate_t");
  }
  if (return_extra) {
      to_return.push_back(alphaHat_t, "alphaHat_t");
      to_return.push_back(betaHat_t, "betaHat_t");
      to_return.push_back(eMatHapSNP_t, "eMatHapSNP_t");      
  }
  if (generate_fb_snp_offsets == true) {
      to_return.push_back(alphaBetaBlocks, "alphaBetaBlocks");
  }
  return(wrap(to_return));  
}











//' @export
// [[Rcpp::export]]
List cpp_read_reassign(
    arma::ivec ord,
    arma::ivec qnameInteger_ord,
    Rcpp::List sampleReadsRaw,
    int verbose,
    arma::ivec readStart_ord,
    arma::ivec readEnd_ord,
    int iSizeUpperLimit
) {
    // ord is 0-based original ordering
    // qnameInteger_ord is (ordered) integer representing reads
  
    // initialize
    int curRead = qnameInteger_ord[0];
    int iReadStart = 0;
    int nRawReads = sampleReadsRaw.size();
    int maxnSNPInRead = 1000;
    std::vector<int> base_bq(maxnSNPInRead);
    std::vector<int> base_pos(maxnSNPInRead); // there shouldnt be this many SNPs
    Rcpp::IntegerVector save_read(nRawReads); // over-sized
    Rcpp::LogicalVector save_this_read_check(nRawReads); // over-sized
    save_this_read_check.fill(false);
    int count = 0;
    int nReadsToSave = 0;
    int nSNPsInRead = -1;
    bool save_this_read;
    int iRead = 0;
    int j, r;
    arma::ivec bqL;
    arma::ivec posL;
    int to_add, k;

    // first loop, define which reads to save
    for (iRead = 0; iRead < nRawReads; iRead++ ) {
        if (qnameInteger_ord[iRead + 1] != curRead) {
            nSNPsInRead = -1;
            save_this_read = true;      
            for(j = iReadStart; j <= iRead; j++) {
                if (j < iRead) {
                    if ((readEnd_ord[j + 1] - readStart_ord[j]) > iSizeUpperLimit) {
                        save_this_read = false;
                    }
                }
            }
            if (save_this_read) {
                save_read(count) = curRead;
                save_this_read_check(iRead) = true;
                count++;
                nReadsToSave++;
            }
            iReadStart = iRead + 1;
            curRead = qnameInteger_ord[iRead + 1]; // + 1
        }
    }

    // second loop, use pre-defined sampleReads
    Rcpp::List sampleReads(nReadsToSave);
    
    curRead = qnameInteger_ord[0];    
    count = 0;
    iReadStart = 0;
    for (iRead = 0; iRead < nRawReads; iRead++ ) {
        if (qnameInteger_ord[iRead + 1] != curRead) {
            nSNPsInRead = -1;
            for(j = iReadStart; j <= iRead; j++) {
                r = ord[j];
                // so say first read is 0-based 0:2
                // want to take reads from ord[0:2]
                Rcpp::List readData = as<Rcpp::List>(sampleReadsRaw[r]);
                arma::ivec bqU = as<arma::ivec>(readData[2]);
                arma::ivec pRU = as<arma::ivec>(readData[3]);
                to_add = int(bqU.size());
                while ((nSNPsInRead + to_add) > maxnSNPInRead) {
                    base_bq.resize(maxnSNPInRead * 2);
                    base_pos.resize(maxnSNPInRead * 2);
                    maxnSNPInRead *= 2;
                }
                for(k = 0; k < to_add; k++) {
                    nSNPsInRead++;
                    base_bq[nSNPsInRead] = bqU[k];
                    base_pos[nSNPsInRead] = pRU[k];	  
                }
            }
            //
            if (save_this_read_check(iRead)) {
                arma::ivec bqL(nSNPsInRead + 1);
                arma::ivec posL(nSNPsInRead + 1);
                for(k = 0; k <= nSNPsInRead; k++) {
                    bqL[k] = base_bq[k];
                    posL[k] = base_pos[k];                    
                }
                //bqL = base_bq.subvec(0, nSNPsInRead);
                //posL = base_pos.subvec(0, nSNPsInRead);
                sampleReads[count] = Rcpp::List::create(nSNPsInRead, 0, bqL, posL);
                count++;
            }
            iReadStart = iRead + 1;
            curRead = qnameInteger_ord[iRead + 1]; // + 1
        }
    }
    
    Rcpp::List to_return = Rcpp::List::create(
        Rcpp::Named("sampleReads") = sampleReads,
        Rcpp::Named("save_read") = save_read
    );
    return to_return;
}




