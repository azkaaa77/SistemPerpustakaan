#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>

#ifdef __MINGW32__
extern "C" int __mingw_vsscanf(const char *buffer, const char *format, va_list argp) {
    return vsscanf(buffer, format, argp);
}
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;

enum AppState { LOGIN, ADMIN_DASHBOARD, USER_HOME };
AppState currentState = LOGIN;

// Forward Declaration
class User;
class Anggota;
class Buku;

// Class User
class User {
protected:
    string idUser, nama, username, role;
public:
    User(string id, string n, string u, string r) : idUser(id), nama(n), username(u), role(r) {}
    virtual ~User() {}
    virtual void tampilkanMenu() = 0; // Pure Virtual (Abstraction)
    string getNama() { return nama; }
    string getUsername() { return username; }
    string getId() { return idUser; }
    string getRole() { return role; }
};

// Class Anggota (Turunan User)
class Anggota : public User {
private:
    string alamat, password;
public:
    Anggota(string id, string n, string u, string almt, string pass)
        : User(id, n, u, "Anggota"), alamat(almt), password(pass) {}
    void tampilkanMenu() override {}
    string getPassword() { return password; }
    string getAlamat() { return alamat; }
};

// Class Buku
class Buku {
private:
    string idBuku, judul, penulis;
    bool isTersedia;
public:
    Buku(string id, string j, string p) : idBuku(id), judul(j), penulis(p), isTersedia(true) {}
    string getId() { return idBuku; }
    string getJudul() { return judul; }
    string getPenulis() { return penulis; }
    bool cekStatus() { return isTersedia; }
    void setStatus(bool status) { isTersedia = status; }
};

// Class Transaksi
class Transaksi {
protected:
    string idTransaksi, tanggal;
public:
    Transaksi(string id, string tgl) : idTransaksi(id), tanggal(tgl) {}
    virtual ~Transaksi() {}
};

// Class Pengembalian (Turunan Transaksi)
class Pengembalian : public Transaksi {
private:
    int dendaPerHari = 2000;
public:
    Pengembalian(string id, string tgl) : Transaksi(id, tgl) {}
    
    // Logika Hitung Denda Otomatis
    void prosesTransaksi(Buku &b, Anggota &a, int hariPinjam, vector<string> &log) {
        int denda = (hariPinjam > 7) ? (hariPinjam - 7) * dendaPerHari : 0;
        b.setStatus(true);
        string msg = "[KEMBALI] " + a.getNama() + " mengembalikan " + b.getJudul();
        if (denda > 0) msg += " (Denda: Rp " + to_string(denda) + ")";
        else msg += " (Tepat Waktu)";
        log.push_back(msg);
    }
};

// Global Variabel
vector<Buku> daftarBuku;
vector<Anggota> daftarAnggota;
vector<string> logAktivitas;
User* currentUser = nullptr; 
int inputHari[1000] = {0};
int menuAdmin = 0;

void DrawLoginPage();
void DrawAdminDashboard();
void DrawUserHome();

//Implementasi fungsi ui

void SetupTokyoNightTheme() {
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.27f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.25f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.48f, 0.64f, 0.97f, 1.00f);
    style.WindowRounding = 8.0f;
}

void DrawLoginPage() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::Begin("LoginScreen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    static char inputUser[32] = "";
    static char inputPass[32] = "";
    static string loginError = "";

    ImGui::SetCursorPos(ImVec2(viewport->Size.x * 0.5f - 100, viewport->Size.y * 0.35f));
    ImGui::BeginGroup();
    ImGui::Text("MNC LIBRARY LOGIN"); ImGui::Spacing();
    ImGui::Text("Username:");
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##user", inputUser, 32);
    ImGui::Text("Password:");
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##pass", inputPass, 32, ImGuiInputTextFlags_Password);
    ImGui::Spacing();

    if (ImGui::Button("LOGIN", ImVec2(200, 35))) {
        if (strcmp(inputUser, "admin") == 0 && strcmp(inputPass, "mncu123") == 0) {
            currentState = ADMIN_DASHBOARD;
            loginError = "";
        } else {
            bool found = false;
            for (auto &ang : daftarAnggota) {
                if (ang.getUsername() == inputUser && ang.getPassword() == inputPass) {
                    currentState = USER_HOME;
                    currentUser = &ang;
                    loginError = "";
                    found = true; break;
                }
            }
            if (!found) loginError = "Username atau Password Salah!";
        }
    }
    if (!loginError.empty()) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", loginError.c_str());
    ImGui::EndGroup();
    ImGui::End();
}

void DrawAdminDashboard() {
    // Sidebar Navigation
    ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos(ImVec2(0, 0)); ImGui::SetWindowSize(ImVec2(200, 720));
    ImGui::Text("MENU ADMIN"); ImGui::Separator();
    if (ImGui::Button("Kelola Buku", ImVec2(-1, 40))) menuAdmin = 0;
    if (ImGui::Button("Kelola Anggota", ImVec2(-1, 40))) menuAdmin = 1;
    if (ImGui::Button("Log Aktivitas", ImVec2(-1, 40))) menuAdmin = 2;
    ImGui::SetCursorPosY(650);
    if (ImGui::Button("LOGOUT", ImVec2(-1, 40))) currentState = LOGIN;
    ImGui::End();

    // Main Content
    ImGui::SetNextWindowPos(ImVec2(210, 10)); ImGui::SetNextWindowSize(ImVec2(1050, 700));
    ImGui::Begin("Main Content", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    
    if (menuAdmin == 0) {
        static char bID[64] = "", bJudul[128] = "", bPen[128] = "", searchBuku[128] = "";
        ImGui::Text("MANAJEMEN DATA BUKU"); ImGui::Separator();
        
        // Form Input
        ImGui::InputText("ID Buku", bID, 64);
        ImGui::InputText("Judul", bJudul, 128);
        ImGui::InputText("Penulis", bPen, 128);
        if (ImGui::Button("Tambah Buku")) {
            daftarBuku.push_back(Buku(bID, bJudul, bPen));
            logAktivitas.push_back("[SISTEM] Admin menambah buku: " + string(bJudul));
        }
        ImGui::Separator();
        
        // Search
        ImGui::Text("Pencarian:"); ImGui::SameLine();
        ImGui::InputText("##searchBuku", searchBuku, 128);

        if (ImGui::BeginTable("TabelBuku", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("ID"); ImGui::TableSetupColumn("Judul"); ImGui::TableSetupColumn("Penulis"); ImGui::TableSetupColumn("Aksi"); ImGui::TableHeadersRow();
            for (int i = 0; i < (int)daftarBuku.size(); i++) {
                // Logika Filter Search
                string judulLower = daftarBuku[i].getJudul();
                string queryLower = searchBuku;
                if (!queryLower.empty() && judulLower.find(queryLower) == string::npos) continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", daftarBuku[i].getId().c_str());
                ImGui::TableSetColumnIndex(1); ImGui::Text("%s", daftarBuku[i].getJudul().c_str());
                ImGui::TableSetColumnIndex(2); ImGui::Text("%s", daftarBuku[i].getPenulis().c_str());
                ImGui::TableSetColumnIndex(3);
                if (ImGui::Button(("Hapus##" + to_string(i)).c_str())) daftarBuku.erase(daftarBuku.begin() + i);
            }
            ImGui::EndTable();
        }
    } else if (menuAdmin == 1) {
        static char aID[64] = "", aNama[128] = "", aUser[64] = "", aAlmt[128] = "", aPass[64] = "";
        ImGui::Text("REGISTRASI ANGGOTA BARU"); ImGui::Separator();
        ImGui::InputText("ID (NIM)", aID, 64);
        ImGui::InputText("Nama", aNama, 128);
        ImGui::InputText("Username", aUser, 64);
        ImGui::InputText("Alamat", aAlmt, 128);
        ImGui::InputText("Password", aPass, 64, ImGuiInputTextFlags_Password);
        if (ImGui::Button("Daftarkan Mahasiswa")) {
            daftarAnggota.push_back(Anggota(aID, aNama, aUser, aAlmt, aPass));
            logAktivitas.push_back("[SISTEM] Anggota baru: " + string(aNama));
        }
    } else {
        ImGui::Text("LOG AKTIVITAS"); ImGui::Separator();
        for (int i = logAktivitas.size() - 1; i >= 0; i--) ImGui::Text("- %s", logAktivitas[i].c_str());
    }
    ImGui::End();
}

void DrawUserHome() {
    static char searchUser[128] = "";
    ImGui::SetNextWindowPos(ImVec2(50, 50)); ImGui::SetNextWindowSize(ImVec2(1180, 620));
    ImGui::Begin("Dashboard User", nullptr, ImGuiWindowFlags_NoResize);
    
    if (currentUser != nullptr) {
        ImGui::Text("Selamat Datang, %s!", currentUser->getNama().c_str());
        Anggota* mhs = static_cast<Anggota*>(currentUser);
        ImGui::Text("📍 Alamat: %s", mhs->getAlamat().c_str());
    }
    ImGui::SameLine(1050);
    if (ImGui::Button("LOGOUT", ImVec2(100, 30))) { currentState = LOGIN; currentUser = nullptr; }
    
    ImGui::Separator();
    ImGui::Text("Cari Buku:"); ImGui::SameLine();
    ImGui::InputText("##searchUser", searchUser, 128);

    if (ImGui::BeginTable("UserTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Judul Buku"); ImGui::TableSetupColumn("Status"); ImGui::TableSetupColumn("Hari"); ImGui::TableSetupColumn("Aksi"); ImGui::TableHeadersRow();
        for (int i = 0; i < (int)daftarBuku.size(); i++) {
            // Search Filter
            if (strlen(searchUser) > 0 && daftarBuku[i].getJudul().find(searchUser) == string::npos) continue;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", daftarBuku[i].getJudul().c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", daftarBuku[i].cekStatus() ? "Tersedia" : "Dipinjam");
            
            ImGui::TableSetColumnIndex(2);
            if (!daftarBuku[i].cekStatus()) {
                ImGui::SetNextItemWidth(60);
                ImGui::InputInt(("##H" + to_string(i)).c_str(), &inputHari[i], 0);
            } else { ImGui::Text("-"); }

            ImGui::TableSetColumnIndex(3);
            if (daftarBuku[i].cekStatus()) {
                if (ImGui::Button(("Pinjam##" + to_string(i)).c_str())) {
                    daftarBuku[i].setStatus(false);
                    logAktivitas.push_back("[USER] Pinjam: " + daftarBuku[i].getJudul());
                }
            } else {
                if (ImGui::Button(("Kembali##" + to_string(i)).c_str())) {
                    // Panggil Class Pengembalian (Polymorphismish)
                    Pengembalian p("TX-KMB", "2026-05-07");
                    Anggota* mhs = static_cast<Anggota*>(currentUser);
                    p.prosesTransaksi(daftarBuku[i], *mhs, inputHari[i], logAktivitas);
                    inputHari[i] = 0;
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

// Main Program
int main() {
    if (!glfwInit()) return -1;
    GLFWwindow *window = glfwCreateWindow(1280, 720, "MNC Library System", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    SetupTokyoNightTheme();

    // Data Awal
    daftarBuku.push_back(Buku("B01", "C++ Programming", "Bjarne S."));
    daftarBuku.push_back(Buku("B02", "Cyber Security 101", "Azka F."));
    daftarAnggota.push_back(Anggota("A001", "Azka Firmansyah", "azka_oke", "Bandung", "mhs123"));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

        switch (currentState) {
            case LOGIN: DrawLoginPage(); break;
            case ADMIN_DASHBOARD: DrawAdminDashboard(); break;
            case USER_HOME: DrawUserHome(); break;
        }

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}