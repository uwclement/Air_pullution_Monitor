class Database {
    static dbName = 'cmuproject';
    static dbHost = 'localhost';
    static dbUsername = 'root';
    static dbUserPassword = '';

    static connect() {
        // One connection through whole application
        if (!this.cont) {
            try {
                this.cont = new PDO(`mysql:host=${this.dbHost};dbname=${this.dbName}`, this.dbUsername, this.dbUserPassword);
            } catch (e) {
                console.error(e.message);
            }
        }
        return this.cont;
    }

    static disconnect() {
        this.cont = null;
    }
}