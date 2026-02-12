-- Insert sample data for testing

-- Insert sample authors
INSERT INTO authors (id, name, bio) VALUES 
(1, 'J.K. Rowling', 'Author of Harry Potter series'),
(2, 'George Orwell', 'Author of 1984 and Animal Farm'),
(3, 'J.R.R. Tolkien', 'Author of The Lord of the Rings'),
(4, 'Stephen King', 'Master of horror fiction'),
(5, 'Agatha Christie', 'Queen of mystery novels');

-- Insert sample categories
INSERT INTO categories (id, name, description) VALUES 
(1, 'Fiction', 'Fictional literature'),
(2, 'Fantasy', 'Fantasy and magical stories'),
(3, 'Science Fiction', 'Science fiction and futuristic stories'),
(4, 'Mystery', 'Mystery and detective novels'),
(5, 'Horror', 'Horror and thriller stories');

-- Insert sample books
INSERT INTO books (id, isbn, title, description, author_id, category_id, price, stock_quantity) VALUES 
(1, '978-0743273565', 'The Great Gatsby', 'A classic American novel', 2, 1, 12.99, 50),
(2, '978-0261103573', 'The Lord of the Rings', 'Epic fantasy adventure', 3, 2, 24.99, 30),
(3, '978-0747532699', 'Harry Potter and the Sorcerer''s Stone', 'The first Harry Potter book', 1, 2, 14.99, 100),
(4, '978-0451524935', '1984', 'Dystopian social science fiction', 2, 3, 13.99, 40),
(5, '978-1501173219', 'The Outsider', 'A supernatural thriller', 4, 5, 16.99, 25),
(6, '978-0007119318', 'Murder on the Orient Express', 'Classic mystery novel', 5, 4, 11.99, 35);

-- Insert sample users
INSERT INTO users (username, password_hash, first_name, last_name, role) VALUES 
('admin', 'password123', 'Admin', 'User', 'admin'),
('staff', 'password123', 'Staff', 'Member', 'staff'),
('user', 'password123', 'Regular', 'User', 'user');

-- Insert sample reviews
INSERT INTO reviews (user_id, book_id, rating, comment, is_verified_purchase) VALUES 
((SELECT id FROM users WHERE username = 'user'), 1, 5, 'Excellent book!', true),
((SELECT id FROM users WHERE username = 'user'), 2, 4, 'Great fantasy adventure', true),
((SELECT id FROM users WHERE username = 'admin'), 3, 5, 'Magical and engaging', false);

-- Insert sample webhooks
INSERT INTO webhooks (user_id, url, events, secret) VALUES 
((SELECT id FROM users WHERE username = 'admin'), 'http://example.com/webhook', '["order.created", "payment.completed"]', 'webhook_secret_123'),
((SELECT id FROM users WHERE username = 'user'), 'http://localhost:3000/hook', '["review.created"]', 'user_webhook_key');

-- Insert system config (for BOLA vulnerability testing)
INSERT INTO system_config (key, value, is_sensitive) VALUES 
('site_name', 'GraphQL Bookstore', false),
('max_order_value', '10000.00', false),
('admin_email', 'admin@bookstore.com', false),
('payment_gateway_key', 'sk_test_secret_key_here', true),
('database_encryption_key', 'super_secret_encryption_key', true);