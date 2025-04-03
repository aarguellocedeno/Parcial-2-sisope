
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
    int time, Aunqueda, arriTime, waitTime = 0, responseTimes = -1, compleTime = -1;
    int queueCola, prioridad; // con esto se asignan lo de las colas y sus prioridadess

    Process(string nombre, int bt, int at, int cola, int pri) {
        etiqueta = nombre; 
        time=bt; 
        Aunqueda = bt; 
        arriTime = at; 
        queueCola = cola; 
        prioridad=pri; 
    }
};

class Cola{
    public:
    string etique;
    int quantumRR;
    deque<Process*> listProcess;
    Cola(string plani, int quantum){ etique = plani; quantumRR =quantum;} // constructor de la cola, recibe el tipo de planificador y un entero que sera el quantum en los rr
    // la funcion agrega, añade un proceso a la cola y lo ordena segun el algoritmo o pues planificador que le corresponda

    void agrega(Process* pro) {
        listProcess.push_back(pro);
        if (etique == "FCFS") {
            sort(listProcess.begin(), listProcess.end(), [](Process* a, Process* b) {
                return a->arriTime < b->arriTime;
            });
        }
    }
    Process* sig(){ 
        if (listProcess.empty()) { return nullptr; }
        else{ return listProcess.front(); }
    };
};


class MLQplani {
    private:
    deque<Cola> queueColas;
    int tiempo = 0;
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
    void ejecutar() {
        while (quedanProcesos()) {
            Process* proc = nullptr;
            Cola* cola_actual = nullptr;
            bool procesoEncontrado = false;
            for (size_t i = 0; i < queueColas.size() && !procesoEncontrado; ++i) {
                Cola& cola = queueColas[i];
                for (size_t j = 0; j < cola.listProcess.size() && !procesoEncontrado; ++j) {
                    Process* p = cola.listProcess[j];
                    if (p->arriTime <= tiempo) {
                        proc = p;
                        cola_actual = &cola;
                        procesoEncontrado = true;
                    }
                }
            }
    
            if (procesoEncontrado) {
                int ejec;
                if (cola_actual->etique == "RR") {
                    ejec = min(cola_actual->quantumRR, proc->Aunqueda); // se busca el minimo entre el tiempo del RR, y el que le queda al proceso
                } else {
                    ejec = proc->Aunqueda;
                }
    
                // Actualizar response time
                if (proc->responseTimes == -1) {
                    proc->responseTimes = tiempo - proc->arriTime;
                }
                tiempo += ejec;
                proc->Aunqueda -= ejec;
                // acá se mira si ya el proceso se acabo, si Si entonces lo elimina de la Cola, caso contrario hago lo de la rotacion o pues cambio de proceso
                if (proc->Aunqueda <= 0) {
                    proc->compleTime = tiempo;
                    proc->waitTime = proc->compleTime - proc->arriTime - proc->time;
                    proFIN.push_back(proc);
                    cola_actual->listProcess.erase(
                        remove(cola_actual->listProcess.begin(), cola_actual->listProcess.end(), proc), 
                        cola_actual->listProcess.end()
                    );
                } 
                else if (cola_actual->etique == "RR") {
                    cola_actual->listProcess.push_back(proc);
                    cola_actual->listProcess.pop_front();
                }
            } 
            else {
                tiempo += 1;// Es para por si no hay procesos listos, avanza tiempo
            }
        }
    }


    // pesta funcion es la que pasa la respuesta a un txt
    void Answer(const string& archivo) {
        double wt1=0,rt1=0,ct1=0,tati=0;
        double cnt =0;
        ofstream f(archivo);
        if (!f) {
            cerr << " No se pudo abrir el archivo "<< endl;
            return;
        }
        f << "#Archivo: mlq001.txt \n";
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
        //cout <<"wt1 = " << 94 << "cnt = "<< 5  << "wt2 "<< 94/5 << endl ;
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

    ifstream entrada("mlq001.txt");
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

    mlq.Answer("mlq001SALIDA.txt"); 

    return 0;
}
