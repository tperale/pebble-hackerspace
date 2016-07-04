const app = require('./appinfo.js');

class Contact {
    constructor (contact) {
        this.obj = contact;
    }

    send (callback) {
        const self = this;

        if (this.obj) {
            const formatedObject = {
                'KEY_TYPE' : app.KEY_CONTACT,

                'KEY_CONTACT_PHONE_NUMBER' : self.obj['phone'],
                'KEY_CONTACT_SIP_ADDRESS' : self.obj['sip'],
                'KEY_CONTACT_IRC' : self.obj['irc'],
                'KEY_CONTACT_TWITTER' : self.obj['twitter'],
                'KEY_CONTACT_FACEBOOK' : self.obj['facebook'],
                'KEY_CONTACT_IDENTICA' : self.obj['identica'],
                'KEY_CONTACT_FOURSQUARE' : self.obj['foursquare'],
                'KEY_CONTACT_EMAIL' : self.obj['email'],
                'KEY_CONTACT_MAILLING_LIST' : self.obj['ml'],
                'KEY_CONTACT_JABBER' : self.obj['jabber'],
                'KEY_CONTACT_ISSUE_MAIL' : self.obj['issue_mail'],
            };

            Pebble.sendAppMessage(formatedObject, () => {
                console.log('SENT : ' + JSON.stringify(formatedObject));
                if (callback) callback(null);
            }, () => {
                console.log('Contact transmission failed ' + JSON.stringify(formatedObject));
                if (callback) callback('Contact transmission failed ' + JSON.stringify(formatedObject));
            });
        } else if (callback) {
            callback(null);
        }
    }
}

exports.Contact = Contact;
