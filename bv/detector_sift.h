#ifndef _BV_DETECTOR_SIFT_H_
#define _BV_DETECTOR_SIFT_H_ 

#include <vector>
#include <utility>
#include <Eigen/Core>
#include <Eigen/LU> 
#include <cmath>
#include "beginvision.h"
#include "filter.h"
#include "image_convert.h"
#include "util.h"


namespace bv {

class DT_Sift {
public:
    DT_Sift(int numOctaves = 4, int S = 3, int minOctave = 0): 
            numOctaves_(numOctaves), minOctave_(minOctave), S_(S) { 
        
        numLevels_ = S_ + 3;
        k = powf(2, 1/S_);
        sigmaNominal_ = 0.5;
        sigma0_ = 1.6; 
    }
    
public:
    int run(Eigen::MatrixXd& img) {
        
        // 0. normlized image
        Eigen::MatrixXd I = img;
        I = I - Eigen::MatrixXd::Ones(img.rows(), img.cols()) * I.minCoeff();
        I = I / I.maxCoeff();    
            
        // 1. upsamle/downsample the original image
        if ( minOctave_ != 0) {
            // TODO 
        }     
    

        // 2. building scale-space image
        buildOctaves(I); 
        
        return BV_OK;
    }

private:
    /*
       The scale space is defined as:
        sigma(o,s) = sigma0 * pow(2, o + s / S) = sigma0 * 2^o * 2^(s/S) 
        s is from smin to smin+numLevle

       I_sigman = g_sqrt( sigman^2 - sigma0^2) * I_sigma0  ( sigman > sigma0) 

       So the diffrent scale step is computed as follow:
       
            sqrt( sigma(o,s+1)^2 - sigma(o,s)^2) =
       =    sqrt( sigma0^2 * 2^(2o) * ( 2^((2s+2)/S) - 2^(2s/S) ))
       =    sigma0 * 2^o * sqrt ( 2^(2s/S) ( 2^(2/S) - 1) )
       =    sigma0 * 2^(o+s/S) * sqrt( 2^(2/S) - 1)

       we can ommit the 2^o in the same octvae.  
    */
    void buildOctaves( Eigen::MatrixXd& I) {
        Eigen::MatrixXd bottomLevel = I;
        double bottomSigma = sigma0_ * powf(2, minOctave_);

        if ( bottomSigma > sigmaNominal_ * powf(2, minOctave_)) {
            double sa = bottomSigma;
            double sb = sigmaNominal_ * powf(2, minOctave_); 
            double sigma = sqrt( sa*sa - sb*sb);
            
            Eigen::MatrixXd temp = bottomLevel;
            std::cout << " bottom sigma = " << bottomSigma << std::endl;
            std::cout << " first  sigma = " << sigma << std::endl;
            Filter::gaussianBlur(bottomLevel, temp, (int)(sigma*3+0.5)*2 + 1, sigma);
            bottomLevel = temp;
        }

        for(int oi = 0; oi < 1; oi++) {
            int currentOctave = oi + minOctave_;
        
            SiftImageOctave octave;
            octave.wid_ = bottomLevel.rows();
            octave.hei_ = bottomLevel.cols();
            octave.images_.push_back( std::pair<double, Eigen::MatrixXd> (bottomSigma, bottomLevel) );
            double lastSigma = bottomSigma;
            Eigen::MatrixXd lastLevel = bottomLevel;
            
            for ( int li = 1; li < numLevels_; li++) {
                double currentSigma = sigma0_ * powf(2, li * 1.0 / S_);
                double diffSigma = sqrt( currentSigma*currentSigma - lastSigma*lastSigma);
                lastSigma = currentSigma;

                Eigen::MatrixXd temp = lastLevel;

                std::cout << " sigma = " << diffSigma << std::endl;
                Filter::gaussianBlur(lastLevel, temp, (int)(currentSigma*3+0.5)*2 + 1, currentSigma);
                
                octave.images_.push_back( std::pair<double, Eigen::MatrixXd> (currentSigma, temp) );
                lastLevel = temp;
            }
             
        }    
    
    
    }


protected:

    typedef struct {
        unsigned int wid_;
        unsigned int hei_;
        std::vector<std::pair<double, Eigen::MatrixXd> > images_;
    } SiftImageOctave;

    // passed from parameters
    int numOctaves_;
    int numLevels_;
    int minOctave_;
    int minLevel_;
    int S_;
    
    // initialed in creator
    double k;
    double sigmaNominal_;
    double sigma0_;

    std::vector<SiftImageOctave> octaves_;
};

}


#endif
