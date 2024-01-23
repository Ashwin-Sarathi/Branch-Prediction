#ifndef SIM_BP_H
#define SIM_BP_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>

typedef struct bp_params{
    unsigned long int  K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int  N;
    char*        bp_name;
}bp_params;

using namespace std;

class Branch {
    public:
        int index_bits_bimodal;
        int index_bits_gshare;
        int index_bits_hybrid;

        int BHR_bits;
        int BHR;

        int sets_bimodal;
        int sets_gshare;
        int sets_hybrid;

        int *BHT_bimodal;
        int *BHT_gshare;
        int *BHT_hybrid;

        int PC;

        int index_bimodal;
        int index_gshare;
        int index_hybrid;

        int number_of_predictions = 0;
        int number_of_mispredictions = 0;
        float misprediction_rate = 0;

        char decision;
        char prediction_bimodal;
        char prediction_gshare;
        char prediction_hybrid;

        string predictor_type;
        string trace;
    
    Branch(int index_bits_bimodal, string predictor_type, int BHR_bits, int index_bits_gshare, int index_bits_hybrid) {
        this->index_bits_bimodal = index_bits_bimodal;
        this->index_bits_gshare = index_bits_gshare;
        this->index_bits_hybrid = index_bits_hybrid;

        this->BHR_bits = BHR_bits;

        this->predictor_type = predictor_type;

        this->sets_bimodal = pow(2, index_bits_bimodal);
        this->sets_gshare = pow(2, index_bits_gshare);
        this->sets_hybrid = pow(2, index_bits_hybrid);

        BHT_layout();
    }   

    void BHT_layout() {
        BHT_bimodal = new int[sets_bimodal];
        for (int i = 0; i < sets_bimodal; i++) {
            BHT_bimodal[i] = 2;
        }

        BHT_gshare = new int[sets_gshare];
        for (int i = 0; i < sets_gshare; i++) {
            BHT_gshare[i] = 2;
        }

        BHT_hybrid = new int[sets_hybrid];
        for (int i = 0; i < sets_hybrid; i++) {
            BHT_hybrid[i] = 1;
        }

        BHR = 0;
    }

    void trace_parse(string trace) {
        number_of_predictions ++;

        istringstream iss(trace);
        iss >> hex >> PC;
        iss >> decision;

        if (predictor_type == "bimodal") {
            find_index_bimodal();            
            bimodal_counter_prediction();
            BHT_update_bimodal();
        }
        else if (predictor_type == "gshare")
        {
            find_index_gshare();
            gshare_counter_prediction();
            BHR_update();
            BHT_update_gshare();
        } 
        else {
            find_index_hybrid();
            find_index_bimodal();
            find_index_gshare();
            bimodal_counter_prediction();
            gshare_counter_prediction();
            hybrid_chooser_counter_prediction();
            chooser_counter_update();
            BHR_update();
        }
    }

    void find_index_bimodal() {
        index_bimodal = (PC >> 2) % sets_bimodal; // Right shift by 2 since LSB bits of PC are always 00
    }

    void find_index_gshare() {
        int temp1;
        temp1 = ((PC >> 2) % sets_gshare);

        int temp2;
        temp2 =( (BHR << (index_bits_gshare - BHR_bits)) % sets_gshare);

        index_gshare = temp1 ^ temp2;
    }

    void find_index_hybrid() {
        index_hybrid = (PC >> 2) % sets_hybrid;
    }

    void bimodal_counter_prediction() {
        if (BHT_bimodal[index_bimodal] <= 1) {
            prediction_bimodal = 'n';
        }
        else {
            prediction_bimodal = 't';
        }

        if (decision != prediction_bimodal) {
            if (predictor_type != "hybrid") {            
                number_of_mispredictions ++;
            }
        }
    }

    void gshare_counter_prediction() {
        if (BHT_gshare[index_gshare] <= 1) {
            prediction_gshare = 'n';
        }
        else {
            prediction_gshare = 't';
        }

        if (decision != prediction_gshare) {
            if (predictor_type != "hybrid") {                
                number_of_mispredictions ++;
            }
        }
    }

    void hybrid_chooser_counter_prediction() {
        if (BHT_hybrid[index_hybrid] <= 1) {
            prediction_hybrid = prediction_bimodal;
            hybrid_misprediction_counter();
            BHT_update_bimodal();
        }
        else {
            prediction_hybrid = prediction_gshare;
            hybrid_misprediction_counter();
            BHT_update_gshare();
        }
    }

    void hybrid_misprediction_counter() {
        if (prediction_hybrid != decision) {
            number_of_mispredictions ++;
        }
    }

    void BHR_update() {
        int BHR_shifted;
        BHR_shifted = BHR >> 1;
        if (decision == 't') {
            BHR = BHR_shifted + pow(2, (BHR_bits - 1));
        }
        else {
            BHR = BHR_shifted;
        }
    }

    void BHT_update_bimodal() {
        if (decision == 't') {
            if (BHT_bimodal[index_bimodal] < 3 ) {
                BHT_bimodal[index_bimodal] ++;
            }                        
        }
        else 
            if (BHT_bimodal[index_bimodal] > 0) {
                BHT_bimodal[index_bimodal] --;
            }
    }

    void BHT_update_gshare() {
        if (decision == 't') {
            if (BHT_gshare[index_gshare] < 3 ) {
                BHT_gshare[index_gshare] ++;
            }                        
        }
        else 
            if (BHT_gshare[index_gshare] > 0) {
                BHT_gshare[index_gshare] --;
            }
    }

    void chooser_counter_update() {
        if (((prediction_bimodal == decision) & (prediction_gshare == decision)) | ((prediction_bimodal != decision) & (prediction_gshare != decision))) {
            return;
        }
        else if ((prediction_bimodal == decision) & (prediction_gshare != decision)) {
            if (BHT_hybrid[index_hybrid] > 0) {
                BHT_hybrid[index_hybrid] --;
            }
        }        
        else {
            if (BHT_hybrid[index_hybrid] < 3) {
                BHT_hybrid[index_hybrid] ++;
            }
        }
    }

    void measurements() {
        float temp;
        temp = static_cast<float>(number_of_mispredictions) / static_cast<float>(number_of_predictions);
        misprediction_rate = temp*100;
        cout << setw(30) << left << " number of predictions:" << number_of_predictions << endl;
        cout << setw(30) << left << " number of mispredictions:" <<  number_of_mispredictions << endl;
        cout << setw(30) << left << " misprediction rate:" << fixed << setprecision(2) << misprediction_rate << "%" << endl;
    }

    void print_BHT () {      
        if (predictor_type == "bimodal") {
            cout << "FINAL BIMODAL CONTENTS" << endl;
            for (int i = 0; i < sets_bimodal; i++) {
                cout << " " << i << "\t" << BHT_bimodal[i] << endl; 
            }
        }
        else if (predictor_type == "gshare") {
            cout << "FINAL GSHARE CONTENTS" << endl;
            for (int i = 0; i < sets_gshare; i++) {
                cout << " " << i << "\t" << BHT_gshare[i] << endl; 
            }           
        }
        else {
            cout << "FINAL CHOOSER CONTENTS" << endl;
            for (int i = 0; i < sets_hybrid; i++) {
                cout << " " << i << "\t" << BHT_hybrid[i] << endl; 
            }

            cout << "FINAL GSHARE CONTENTS" << endl;
            for (int i = 0; i < sets_gshare; i++) {
                cout << " " << i << "\t" << BHT_gshare[i] << endl; 
            }  

            cout << "FINAL BIMODAL CONTENTS" << endl;
            for (int i = 0; i < sets_bimodal; i++) {
                cout << " " << i << "\t" << BHT_bimodal[i] << endl; 
            }
        }
    }

    void cleanup() {    
        delete[] BHT_bimodal;
        delete[] BHT_gshare;     
        delete[] BHT_hybrid;
    }
};

#endif
