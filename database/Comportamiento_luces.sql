CREATE TABLE comportamiento_luces (
    id INT PRIMARY KEY AUTO_INCREMENT,
    red INT NOT NULL CHECK (red>=0 AND red<=255),
    green INT NOT NULL CHECK (green>=0 AND green<=255),
    blue INT NOT NULL CHECK (blue>=0 AND blue<=255),
    brillo INT NOT NULL CHECK (red>=0 AND red<=255),
    id_usuario INT,
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id)
);