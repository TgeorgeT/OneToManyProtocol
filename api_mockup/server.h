

int init(uint16_t port)
{
    // primeste un port
    // deschide un socket de udp pe portul specificat
}

void listen_server(int32_t socket)
{
    // primeste socketul creat prin bind server si asteapta pachete de la clienti
    // clientii vor trimite un pachet prin udp? in care specifica tipul conexiunii
    // iar serverul ii va raspunde cu un pachet in care specifica portul pe care se
    // va desfasura conexiunea efctiv
    // clientul va trebui sa mai raspunda cu un pachet in care sa confirme ca a primit portul
    //(un fel de tcp handshake)
    // tot aici la primirea pachetului de confirmare se va crea canalul
    // efectiv de comunicatie si va inserat in structura care gestioneaza toate canalele
}

void get_channels_info()
{
    // intoarce informatii in legatura cu fiecare canal de comunicatie
    // ip, port, tip
}

void get_channel_by_ip(int32_t ip)
{
    // primeste un ip si returneaza canalul corespunzator
}

void send_message_to_channel(char *buf, int32_t len, uint_16 channel_number)
{
    // trimite un mesaj catre un canal
}

void send_message_to_all(char *buf, int32_t len)
{
    // trimite un mesaj catre toti clientii
}

void close_channel(uint16_t channel_number)
{
    // inchide un canal
}

void close_all()
{
    // inchide toate canalele
}