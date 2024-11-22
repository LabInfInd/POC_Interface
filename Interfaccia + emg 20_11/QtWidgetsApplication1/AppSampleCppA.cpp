///[ ----------------------------------------------------------------------------
//[ BioDAQ (BDQ)
//[
//[ (c) 2012 BTS SpA
//[ ----------------------------------------------------------------------------
//[
//[ $Id: AppSampleCppA.cpp 2529 2017-05-05 12:44:56Z GENALE $
//[ ----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// This sample illustrates how to make a data acquisition with a RT100 device
// with a set (>=1) of configured EMG probes.
// -----------------------------------------------------------------------------


// Include standard libraries
#include "stdafx.h"
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")



// Include BioDAQ template libraries (registration has been done!)

#import "mscorlib.tlb"
#import "bts.biodaq.drivers.tlb"
#import "bts.biodaq.core.tlb"


//
// NOTE: In the folder where .exe is built, we have to copy the BioDAQ libraries + log4net.dll + TdfAccess100.dll + 
//       msvcr100.dll + msvcp100.dll (+ msvcr100d.dll + msvcp100d.dll, if Configuration == DEBUG)
//


// Use namespaces
using namespace std;
using namespace mscorlib;
using namespace bts_biodaq_core;
using namespace bts_biodaq_drivers;


void startTcpServer()
{
    WSADATA wsaData;
    SOCKET listeningSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
    sockaddr_in serverAddr = { 0 }, clientAddr = { 0 };
    int clientAddrLen = sizeof(clientAddr);

    // Inizializza Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore: impossibile inizializzare Winsock.\n");
        return;
    }

    // Crea il socket in ascolto
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == INVALID_SOCKET) {
        printf("Errore: impossibile creare il socket.\n");
        WSACleanup();
        return;
    }

    // Configura l'indirizzo del server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);  // Porta in ascolto

    // Associa il socket all'indirizzo e alla porta
    if (bind(listeningSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Errore: impossibile associare il socket.\n");
        closesocket(listeningSocket);
        WSACleanup();
        return;
    }

    // Metti il socket in ascolto
    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Errore: impossibile mettere in ascolto il socket.\n");
        closesocket(listeningSocket);
        WSACleanup();
        return;
    }

    printf("Server in ascolto sulla porta 12345...\n");

    while (true) {
        // Accetta una connessione
        clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("Errore: impossibile accettare la connessione.\n");
            continue;
        }

        printf("Connessione stabilita con un client.\n");

        char buffer[1024] = { 0 };
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            printf("Messaggio ricevuto: %s\n", buffer);

            // Comandi dal client
            if (strcmp(buffer, "Start") == 0) {
                printf("Acquisizione avviata.\n");
            }

            else if (strcmp(buffer, "Stop") == 0) {
                printf("Acquisizione arrestata.\n");
                break; // Esci dal loop per terminare il server
            }

            // Rispondi al client
            const char* response = "Comando ricevuto";
            send(clientSocket, response, strlen(response), 0);
        }

        closesocket(clientSocket); // Chiude la connessione con il client
    }

    printf("Server arrestato.\n");
    closesocket(listeningSocket);
    WSACleanup();
}

//    // Accetta una connessione
//    clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &clientAddrLen);
//    if (clientSocket == INVALID_SOCKET) {
//        printf("Errore: impossibile accettare la connessione.\n");
//        closesocket(listeningSocket);
//        WSACleanup();
//        return;
//    }
//
//    printf("Connessione stabilita con un client.\n");
//
//    // Riceve dati dal client
//    char buffer[1024] = { 0 };
//
//    bool running = true;
//
//    while (running) {
//        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
//        if (bytesRead > 0) {
//            buffer[bytesRead] = '\0';
//            printf("Comando ricevuto: %s\n", buffer);
//
//            // Gestisci i comandi ricevuti
//            if (strcmp(buffer, "START") == 0) {
//                printf("Avvio della registrazione...\n");
//                BioDAQExitStatus errCode = ptrBioDAQ->Record();
//                if (errCode != BioDAQExitStatus_Success) {
//                    printf("Errore durante l'avvio della registrazione.\n");
//                }
//                else {
//                    printf("Registrazione avviata con successo.\n");
//                }
//            }
//            else if (strcmp(buffer, "STOP") == 0) {
//                printf("Interruzione della registrazione...\n");
//                BioDAQExitStatus errCode = ptrBioDAQ->Stop();
//                if (errCode != BioDAQExitStatus_Success) {
//                    printf("Errore durante l'interruzione della registrazione.\n");
//                }
//                else {
//                    printf("Registrazione interrotta con successo.\n");
//                }
//                running = false;  // Termina il loop
//            }
//        }
//    }
//
//    // Chiudi i socket
//    closesocket(clientSocket);
//    closesocket(listeningSocket);
//    WSACleanup();
//}


int _tmain(int argc, _TCHAR* argv[])
{

    // Make sure to add USES_CONVERSION for using ATL string conversion macros
    USES_CONVERSION;

    // Make sure to add the CoInitializeEx() call when the application loads
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    printf("Inizializzazione del server TCP/IP...\n");
    // Avvia il server TCP/IP per ricevere messaggi
    startTcpServer();
    printf("Server TCP/IP terminato. Inizializzo il dispositivo BioDAQ...\n");

    printf("Create a USB COM port where BioDAQ device is connected...\n");

    // In this example it always use the SMART Pointer structure to define objects exposed 
    // by BioDAQ libraries (COM interfaces)

    // Create a COM Port with a given number 
    IPortCOMPtr ptrCOMPort(__uuidof(PortCOM));
    ptrCOMPort->Number = 7;      // set the COM port number where BioDAQ device is connected (use Windows device manager tool)

    // Create the ports List
    IPortListPtr ptrPortList(__uuidof(PortList));
    // Get the IPort interface of the ptrCOMPort
    IPort* pPort;
    ptrCOMPort.QueryInterface(__uuidof(IPort), &pPort);

    // Check the number of ports in the list before insert the new one
    long count = ptrPortList->Count;

    // Insert the new COM port at first position (position zero)
    ptrPortList->Insert(0, pPort);

    // Do some check: number of ports and the port index in the list
    count = ptrPortList->Count;
    long index = ptrPortList->IndexOf(pPort);

    // Create BM View list and BM View objects
    IBMViewListPtr bmViewList;
    IBMViewPtr  bmView;
     //printf("Numero Sensori Connessi: %ld\n", sensorCount); 
        printf("Initializing BioDAQ device...\n");

    //// Instantiate the BioDAQ 
    IBioDAQPtr ptrBioDAQ(__uuidof(BioDAQ));
    // Init the system with the created list of ports
    BioDAQExitStatus errCode = ptrBioDAQ->Init(ptrPortList);
    if (BioDAQExitStatus_Success != errCode)
    {
        printf("Failed: unable to initialize BioDAQ device on USB COM port %d.\n", ptrCOMPort->Number);
        printf("Press a key to exit...\n");
        _gettch();
        return 0;
    }


    // Get the list of BMs 
    bmViewList = ptrBioDAQ->BmsView;

    // Do some check: number of BMs
    long nBMCounts = bmViewList->Count;



    // Get a BM from the list (position zero)
    if (nBMCounts > 0)
    {
        printf("Numero Moduli Base: %d\n", nBMCounts);
        //printf("Numero Moduli Base: %d\n", nBMCounts2);
        bmView = bmViewList->GetItem(0);

        SAFEARRAY* serial = bmView->Serial;
    }

    // Get the active protocol
    IProtocolPtr protocol;
    protocol = ptrBioDAQ->ActiveProtocol;


    long protocolItems = protocol->Count();
    //printf("%d\n", protocolItems);

    // Get the BioDAQ state
    bts_biodaq_core::BioDAQState bioDAQState = ptrBioDAQ->State;

    // Instantiate a DataSink with the provided factory and init it 
    // The type of this DataSink is "Disk" so that it implements the generic IDataSink interface
    // and also the specialized IDiskSink interface
    ISinksFactory* ptrSinksFactory = ptrBioDAQ->SinksFactory;
    IDataSink* pDataSink = ptrSinksFactory->CreateSink(bts_biodaq_core::SinkType::SinkType_Disk);
    pDataSink->Init();
    // Get temporary folder
    char szTempFolder[_MAX_PATH];
    ZeroMemory(szTempFolder, _MAX_PATH * sizeof(char));
    GetTempPathA(_MAX_PATH, szTempFolder);

    // Set trial file name (.bin) on temporary folder
    char szBinFileName[_MAX_PATH];
    ZeroMemory(szBinFileName, _MAX_PATH * sizeof(char));
    strcpy(szBinFileName, szTempFolder);
    strcat(szBinFileName, "trialtest00.bin");

    // Get the specialized IDiskSink interface from the just created DataSink object
    IDiskSinkPtr ptrDiskSink;
    pDataSink->QueryInterface(__uuidof(IDiskSink), (void**)&ptrDiskSink);
    //ptrDiskSink->FileName = "trialtest00.bin";
    ptrDiskSink->FileName = A2BSTR(szBinFileName);

    // Add the DiskSink to the list of the sinks (a collection of data sinks always contains the QueueSink object)
    HRESULT hResult = ptrBioDAQ->Sinks->Add(pDataSink);
    long nSinks = ptrBioDAQ->Sinks->Count;
    // Do some check: number of sinks
    if (nSinks > 0)
    {
        // Get the specialized IQueueSink interface from generic IDataSink interface
        IDataSink* pDataSinkTmp = ptrBioDAQ->Sinks->GetItem(0);
        IQueueSinkPtr ptrQueueSink;
        pDataSinkTmp->QueryInterface(__uuidof(IQueueSink), (void**)&ptrQueueSink);
        // Retrieve the queue size for the first channel (channel index = 0)
        int queueSize0 = ptrQueueSink->QueueSize(0);
    }

    // Set up the channels
    IChannelViewListPtr chViewList;
    bts_biodaq_core::IChannelPtr  channel;

    // Get the channels view list
    chViewList = ptrBioDAQ->ChannelsView;
    long nChannels = chViewList->Count;
    // printf("%d\n", nChannels);
        // Set properties for all channels
        chViewList->SetEMGChannelsRangeCode(EMGChannelRangeCodes_Gain1_5mV);   // EMG channel range
    chViewList->SetEMGChannelsSamplingRate(SamplingRate_Rate1KHz);         // Sampling rate: 1 kHz
    chViewList->SetEMGChannelsCodingType(CodingType_Raw);
    chViewList->SetEMGChannelsCompression(true);                           // Coding type: ADPCM


    printf("Arming BioDAQ device...\n");

    // Arm the BioDAQ
    errCode = ptrBioDAQ->Arm();
    if (BioDAQExitStatus_Success != errCode)
    {
        printf("Failed: unable to arm BioDAQ device\n");
        printf("Press a key to exit...\n");
        _gettch();

        // roll-back
        ptrBioDAQ->Reset();
        ptrBioDAQ->Release();
        bmViewList->Release();
        chViewList->Release();
        ptrCOMPort->Release();

        return 0;
    }

    // --------------------------------------------------------------------------
    ISensorViewDictionaryPtr ptrSensorViewDictionary(__uuidof(SensorViewDictionary));
    ptrSensorViewDictionary = ptrBioDAQ->SensorsView;
    ISensorViewDictionaryPtr sensorsView = ptrBioDAQ->SensorsView;
    long sensorCount = sensorsView->Count;
    printf("Numero Sensori Associati al BM che e' possibile utilizzare: %ld\n", sensorCount);
    printf("Retrieve information of EMG sensor from BioDAQ device...\n");
    long activeSensorCount = 0;

    // Retrieve AUX sensor information
    ISensorViewPtr sensor;

    ISensorViewDictionaryEnumPtr ptrSensorViewDictionaryEnum;
    ptrSensorViewDictionaryEnum = ptrSensorViewDictionary->GetEnumerator();


    for (int i = 0; i < sensorCount; ++i) {
        bool result = (ptrSensorViewDictionaryEnum->MoveNext() == VARIANT_TRUE);
        if (!result) {
            printf("Errore nell'ottenere le informazioni della sonda %d.\n", i + 1);
            continue;
        }

        sensor = ptrSensorViewDictionaryEnum->Current;

        bool isConnected = sensor->GetConnected();

        // Incrementa il contatore solo se la sonda è effettivamente connessa
        if (isConnected) {
            activeSensorCount++;
        }
    }

    printf("Numero di sonde effettivamente connesse e accese: %ld\n", activeSensorCount);
    printf("Starting BioDAQ device...\n");

    //printf("Inizializzazione del server TCP/IP...\n");
    //// Avvia il server TCP/IP per ricevere messaggi
    //startTcpServer(ptrBioDAQ);
    //printf("Server TCP/IP terminato. Inizializzo il dispositivo BioDAQ...\n");

    // Start the acquisition
    errCode = ptrBioDAQ->Start();
    if (BioDAQExitStatus_Success != errCode)
    {
        printf("Failed: unable to arm BioDAQ device\n");
        printf("Press a key to exit...\n");
        _gettch();

        // roll-back
        ptrBioDAQ->Reset();
        ptrBioDAQ->Release();
        bmViewList->Release();
        chViewList->Release();
        ptrCOMPort->Release();

        return 0;
    }

    // Sleep a bit
    Sleep(2000);
    printf("Recording data...\n");



    errCode = ptrBioDAQ->Record();
    if (BioDAQExitStatus_Success != errCode)
    {
        printf("Failed: unable to record data\n");
    }

    BattLevel a;
    SAFEARRAY* address_get;
    bool isConn;
    bool stopRecording = false;
    for (int j = 0; j < 50 && !stopRecording; j++)
    {
        ptrSensorViewDictionaryEnum->Reset();

        for (int i = 0; i <= activeSensorCount - 1; i++)
        {
            printf("\n");

            bool bResult = (ptrSensorViewDictionaryEnum->MoveNext() == VARIANT_TRUE) ? true : false;
            if (!bResult)
            {
                printf("Failed: invalid operation...\n");
                printf("Press a key to exit...\n");
                _gettch();

                // roll-back
                ptrBioDAQ->Reset();
                ptrBioDAQ->Release();
                bmViewList->Release();
                ptrCOMPort->Release();
                return 0;
            }

            ptrBioDAQ->UpdateStatusInfo();
            sensor = ptrSensorViewDictionaryEnum->Current;
            printf("%s\n", address_get);

            a = sensor->GetBattLevel();
            isConn = sensor->GetConnected();

            const char* battLevelStr;

            switch (a) {
            case 1:
                battLevelStr = "Livello batteria inferiore al 25% della massima carica";
                break;
            case 2:
                battLevelStr = "Livello batteria compreso tra il 25% ed il 50% della massima carica";
                break;
            case 3:
                battLevelStr = "Livello batteria compreso tra il 50% ed il 75% della massima carica";
                break;
            case 4:
                battLevelStr = "Livello batteria superiore al 75% della massima carica";
                break;
            default:
                battLevelStr = "Livello di batteria sconosciuto";
                break;
            }

            const char* isConnStr;
            switch (isConn) {
            case 0:
                isConnStr = "Sonda NON Connessa";
                break;
            case 1:
                isConnStr = "Sonda Connessa";
                break;
            default:
                isConnStr = "Livello di connessione sconosciuto";
                break;
            }


            if (isConn == 0)
            {
                errCode = ptrBioDAQ->Stop();

                if (BioDAQExitStatus_Success != errCode)
                {

                    printf("Failed: unable to stop BioDAQ device\n");
                    printf("%d\n", errCode);
                }
                printf("La registrazione viene interrotta a causa di un'interruzione nella connessione della sonda %d..\n", i + 1);
                stopRecording = true;  // Imposta il flag per fermare il ciclo esterno
                break;  // Esci dal ciclo interno

            }


            printf("Sonda %d: %s - %s\n", i + 1, isConnStr, battLevelStr);
        }
        // --------------------------------------------------------------------------
        // 


        // Sleep a bit
        printf("\n");
        Sleep(1000);
    }

    printf("Stopping BioDAQ device...\n");

    // Stop acquisition
    errCode = ptrBioDAQ->Stop();
    if (BioDAQExitStatus_Success != errCode)
    {

        printf("Failed: unable to stop BioDAQ device because the recording was stopped previously due to a probe disconnection\n");
        //printf("%d\n",errCode);
    }

    // Wait ... just a bit
    //Sleep(250);
    Sleep(1000);

    // Data convertion to BTS custom TDF format
    // Instantiate the TrialReader object in order to the read the acquired data
    ITrialReaderPtr ptrTrialReader(__uuidof(TrialReader));
    ptrTrialReader->FileName = ptrDiskSink->FileName;

    char szExportedFileName[_MAX_PATH];
    ZeroMemory(szExportedFileName, _MAX_PATH * sizeof(char));
    strcpy(szExportedFileName, szTempFolder);
    strcat(szExportedFileName, "test6.tdf");

    // Instantiate the TDF exporter setting up the trial reader
    IExporterPtr TDFExporter(__uuidof(TDFExporter));
    TDFExporter->TrialReader = ptrTrialReader;
    //TDFExporter->ExportedFile = "test.tdf";
    TDFExporter->ExportedFile = A2BSTR(szExportedFileName);

    char szFileName[_MAX_PATH];
    ZeroMemory(szFileName, _MAX_PATH * sizeof(char));
    strncpy_s(szFileName, OLE2A(ptrTrialReader->FileName), _MAX_PATH - 1);

    printf("Save data on file %s\n", szExportedFileName);

    // Export the acquired data to the TDF format
    TDFExporter->ExportData();

    // Get the BioDAQ state
    bioDAQState = ptrBioDAQ->State;
    if (BioDAQState_Ready != bioDAQState)
    {
        printf("Error on state for BioDAQ device.\n");
    }
    // Reset the system

    printf("Reset objects...\n");

    // Release BMs view list
    bmViewList->Release();
    // Release channels view list
    chViewList->Release();

    // Reset BioDAQ
    ptrBioDAQ->Reset();

    // Release BioDAQ
    ptrBioDAQ->Release();

    // Clear ports list
    ptrPortList->Clear();
    // Release COM port
    ptrCOMPort->Release();

    printf("Press a key to exit...\n");
    _gettch();

    CoUninitialize();
    return 0;
}
