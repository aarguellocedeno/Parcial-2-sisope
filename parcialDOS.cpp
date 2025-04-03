
#include <iostream>
#include <fstream>
#include <deque>
#include <algorithm>
#include <iomanip>
#include <sstream>
using namespace std;

class Process{
    public:
    string etiqueta;
    float time, Aunqueda, arriTime, waitTime = 0, responseTimes = -1, compleTime = -1;
    int queueCola, prioridad; // con esto se asignan lo de las colas y sus prioridadess

    Process(string nombre, float bt, float at, int cola, int pri) : etiqueta(nombre), time(bt), Aunqueda(bt), arriTime(at), queueCola(cola), prioridad(pri) {}
};

class Cola{
    public:
    string etique;
    int quantumRR;
    deque<Process*> listProcess;
    Cola(string plani, int quantum): etique(plani), quantumRR(quantum){} // constructor de la cola, recibe el tipo de planificador y un entero que sera el quantum en los rr
    // la funcion agrega, añade un proceso a la cola y lo ordena segun el algoritmo o pues planificador que le corresponda
    void agrega(Process* pro){
        bool menor = false;
        listProcess.push_back(pro);
        sort(listProcess.begin(), listProcess.end(), [this](Process* uno, Process* dos){
            if (etique=="RR"){
                return uno->prioridad > dos ->prioridad;
            }else if(etique=="FCFS"){
                return uno->arriTime > dos ->arriTime;
            }
        } );
    }
    Process* sig(){ 
        if (listProcess.empty()) { return nullptr; }
        else{ return listProcess.front(); }
    };
};


class MLQplani {
    deque<Cola> queueColas;
    float tiempo = 0;
    deque<Process*> proFIN;

    //este verifica si todavia quedan procesos en alguna cola
    bool quedanProcesos() {
        return any_of(queueColas.begin(), queueColas.end(), [](Cola& c) { return !c.listProcess.empty(); });
    }

    public:
    MLQplani(deque<pair<string, int>>& config) {
        for (size_t i = 0; i < config.size(); i++) {
            pair<string, int> par = config[i];
            queueColas.emplace_back(par.first, par.second);
        }
    } 

    //añade un proceso a la cola que le corresponda
    void agregarProceso(Process* proc) {
        if (proc->queueCola - 1 < queueColas.size()) {queueColas[proc->queueCola - 1].agrega(proc);}
    }

    // hace como tal el trabajo de calcular todos los valores que se piden y se va a ejecutar mientras queden procesos
    void ejecutar() {
        while (quedanProcesos()) {
            Process* proc = nullptr;
            Cola* cola_actual = nullptr;
            bool procesoEncontrado = false;
            for (auto& cola : queueColas) {
                for (auto& p : cola.listProcess) {
                    if (p->arriTime <= tiempo && !procesoEncontrado) { 
                        proc = p;
                        cola_actual = &cola;
                        procesoEncontrado = true;
                    }
                }
            }
            if (proc->responseTimes == -1) {proc->responseTimes = tiempo - proc->arriTime;}
            float ejec;
            if (cola_actual->etique == "RR") {  ejec = min((float)cola_actual->quantumRR, proc->Aunqueda);
            } else {
                ejec = proc->Aunqueda;
            }
            tiempo += ejec;
            proc->Aunqueda -= ejec;

            // aca verifico que si Aunqueda es <=0, significa que el proceso ya se acabo, entonces lo eliminamos de la lista de procesos, 
            // pero si no se ha terminado y es RR, pues se rota, osea se manda al final de la fila
            if (proc->Aunqueda <= 0) {
                proc->compleTime = tiempo;
                proc->waitTime = proc->compleTime - proc->arriTime - proc->time;
                proFIN.push_back(proc);
                cola_actual->listProcess.erase(
                    remove(cola_actual->listProcess.begin(), cola_actual->listProcess.end(), proc), cola_actual->listProcess.end()
                );
            } else if (cola_actual->etique == "RR") {
                rotate(cola_actual->listProcess.begin(), cola_actual->listProcess.begin() + 1, cola_actual->listProcess.end());
            }
        }
    }

    // pesta funcion es la que pasa la respuesta a un txt
    void Answer(const string& archivo) {
        float wt1=0,rt1=0,ct1=0,tati=0;
        float cnt =0;
        float wt2=0,rt2=0,ct2=0,tat2=0;
        ofstream f(archivo);
        if (!f) {
            cerr << " No se pudo abrir el archivo "<< endl;
            return;
        }
        f << "#Archivo: mlq003.txt \n";
        f << "# etiqueta;BT;AT;Q;Pr;WT;CT;RT;TAT\n";
        for (size_t i = 0; i < proFIN.size(); i++) {
            Process* p = proFIN[i];
            float tat = p->compleTime - p->arriTime;
            f << p->etiqueta << ";" << p->time << ";" << p->arriTime << ";" << p->queueCola << ";" << p->prioridad << ";" << p->waitTime << ";" << p->compleTime << ";" << p->responseTimes << ";" << tat << "\n";
            wt1 += p->waitTime ;
            rt1 += p->responseTimes;
            ct1 += p->compleTime;
            tati += tat;
            cnt += 1;
        }

        wt2  = wt1 / cnt;
        //cout <<"wt1 = " << 94 << "cnt = "<< 5  << "wt2 "<< 94/5 << endl ;
        rt2 = rt1/cnt;
        ct2 = ct1/cnt;
        tat2 = tati/cnt;
        cout << wt2 <<rt2 << ct2<<tat2 << cnt;
        // nose pq no se hace bien la division, incluso cuando imprimi cout << 94/5, me seguia dando un valor incorrecto
        f << "\n"<<"WT = "<<wt1/cnt<< ";" << "CT = "<<ct1/cnt<< ";"<< "RT = "<<rt1/cnt<< ";"<< "TAT = "<<tati/cnt<< ";";
    }
};


int main() {
    deque<pair<string, int>> planini = {
        {"RR", 3},   
        {"RR", 5},   
        {"FCFS", 0}  
    };

    MLQplani mlq(planini); 

    ifstream entrada("mlq003.txt");
    if (!entrada) {
        cerr << "Error al abrir el archivo de entrada." << endl;
        return 1;
    }

    string linea;
    while (getline(entrada, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        replace(linea.begin(), linea.end(), ';', ' ');
        stringstream ss(linea);
        string etiq;
        float bt, at;
        int q, p;
        if (ss >> etiq >> bt >> at >> q >> p) {
            mlq.agregarProceso(new Process(etiq, bt, at, q, p));
        }
    }

    mlq.ejecutar();

    mlq.Answer("mlq003SALIDA.txt"); 

    return 0;
}