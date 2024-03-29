CREATE TABLE product_category (
    id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL
);

CREATE TABLE product (
    id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    category_id INT,
    FOREIGN KEY (category_id) REFERENCES product_category(id)
);

CREATE TABLE variation (
    id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL
);

CREATE TABLE variation_option (
    id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    variation_id INT,
    FOREIGN KEY (variation_id) REFERENCES variation(id)
);

CREATE TABLE product_configuration (
    id INT PRIMARY KEY,
    product_id INT,
    variation_option_id INT,
    FOREIGN KEY (product_id) REFERENCES product(id),
    FOREIGN KEY (variation_option_id) REFERENCES variation_option(id)
);

CREATE TABLE product_item (
    id INT PRIMARY KEY,
    product_configuration_id INT,
    FOREIGN KEY (product_configuration_id) REFERENCES product_configuration(id)
);

CREATE TABLE promotion_category (
    id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL
);

CREATE TABLE promotion (
    id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    discount DECIMAL(5,2),
    promotion_category_id INT,
    FOREIGN KEY (promotion_category_id) REFERENCES promotion_category(id)
);
