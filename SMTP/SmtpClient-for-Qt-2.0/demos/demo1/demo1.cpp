#include <QtCore>

#include "../../../Include/SmtpMime"
#include "../demo_vars.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // This is a first demo application of the SmtpClient for Qt project

    // Now we create a MimeMessage object. This is the email.

    MimeMessage message;

    EmailAddress sender(SENDER_EMAIL, SENDER_NAME);
    message.setSender(sender);

    EmailAddress to(RECIPIENT_EMAIL, RECIPIENT_NAME);
    message.addRecipient(to);

    EmailAddress cc(SENT_EMAIL, SENT_FOLDER);
    message.addRecipient(cc);

    message.setSubject("SmtpClient for Qt - Test");


    MimeText text;

    text.setText("Hi,\nThis is a simple email message.\n");

    // Now add it to the mail

    message.addPart(&text);

    // Now we need to create a MimeHtml object for HTML content
    MimeHtml html;

    html.setHtml("<a href=\"https://google.ca\">This is a link to a possible match for xxxxx:</a>");
//    html.setContentId("contentID");
//    html.setContentName("contentName");
//    html.setContentType("links");
    message.addPart(&html);

    // Now we can send the mail
    SmtpClient smtp(SMTP_SERVER, 465, SmtpClient::SslConnection);

    smtp.connectToHost();
    if (!smtp.waitForReadyConnected()) {
        qDebug() << "Failed to connect to host!";
        return -1;
    }

    smtp.login(SENDER_EMAIL, SENDER_PASSWORD);
    if (!smtp.waitForAuthenticated()) {
        qDebug() << "Failed to login!";
        return -2;
    }

    smtp.sendMail(message);
    if (!smtp.waitForMailSent()) {
        qDebug() << "Failed to send mail!";
        return -3;
    }

    smtp.quit();

}
