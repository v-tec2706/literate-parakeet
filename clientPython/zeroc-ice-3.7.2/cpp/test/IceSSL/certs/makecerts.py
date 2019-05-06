#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os, sys, socket, getopt

try:
    import IceCertUtils
except:
    print("error: couldn't find IceCertUtils, install `zeroc-icecertutils' package "
          "from Python package repository")
    sys.exit(1)

if not IceCertUtils.CertificateUtils.opensslSupport:
    print("openssl is required to generate the test certificates")
    sys.exit(1)

def usage():
    print("Usage: " + sys.argv[0] + " [options]")
    print("")
    print("Options:")
    print("-h               Show this message.")
    print("-d | --debug     Debugging output.")
    print("--clean          Clean the CA database first.")
    print("--force          Re-save all the files even if they already exists.")
    sys.exit(1)

#
# Check arguments
#
debug = False
clean = False
force = False
try:
    opts, args = getopt.getopt(sys.argv[1:], "hd", ["help", "debug", "clean", "force"])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

for (o, a) in opts:
    if o == "-h" or o == "--help":
        usage()
        sys.exit(0)
    elif o == "-d" or o == "--debug":
        debug = True
    elif o == "--clean":
        clean = True
    elif o == "--force":
        force = True

home = os.path.join(os.path.dirname(os.path.abspath(__file__)), "db")
homeca1 = os.path.join(home, "ca1")
homeca2 = os.path.join(home, "ca2")
if not os.path.exists("db"):
    os.mkdir(home)
    os.mkdir(homeca1)
    os.mkdir(homeca2)

if clean:
    for h in [homeca1, homeca2]:
        IceCertUtils.CertificateFactory(home=h).destroy(True)

# Create 2 CAs, the DSA ca is actually ca1 but with a different the DSA key generation algorithm.
ca1 = IceCertUtils.CertificateFactory(home=homeca1, cn="ZeroC Test CA 1", ip="127.0.0.1", email="issuer@zeroc.com")
ca2 = IceCertUtils.CertificateFactory(home=homeca2, cn="ZeroC Test CA 2", ip="127.0.0.1", email="issuer@zeroc.com")
dsaca = IceCertUtils.OpenSSLCertificateFactory(home=ca1.home, keyalg="dsa", keysize=1024)

#
# Export CA certificates
#
if force or not os.path.exists("cacert1.pem"): ca1.getCA().save("cacert1.pem")
if force or not os.path.exists("cacert2.pem"): ca2.getCA().save("cacert2.pem")
if force or not os.path.exists("cacert1.der"): ca1.getCA().save("cacert1.der")
if force or not os.path.exists("cacerts.pem"):
    pem = ""
    with open("cacert1.pem", "r") as f: pem += f.read()
    with open("cacert2.pem", "r") as f: pem += f.read()
    with open("cacerts.pem", "w") as f: f.write(pem);

# Also export the ca2 self-signed certificate, it's used by the tests to test self-signed certificates
if force or not os.path.exists("cacert2_pub.pem"): ca2.getCA().save("cacert2_pub.pem")
if force or not os.path.exists("cacert2_priv.pem"): ca2.getCA().saveKey("cacert2_priv.pem")
if force or not os.path.exists("cacert2.p12"): ca2.getCA().save("cacert2.p12", addkey=True)

# Create intermediate CAs
cai1 = ca1.getIntermediateFactory("intermediate1")
if not cai1:
    cai1 = ca1.createIntermediateFactory("intermediate1", cn = "ZeroC Test Intermediate CA 1")
cai2 = cai1.getIntermediateFactory("intermediate1")
if not cai2:
    cai2 = cai1.createIntermediateFactory("intermediate1", cn = "ZeroC Test Intermediate CA 2")

#
# Create certificates (CA, alias, { creation parameters passed to ca.create(...) })
#
certs = [
    (ca1, "s_rsa_ca1",       { "cn": "Server", "ip": "127.0.0.1", "dns": "server", "serial": 1 }),
    (ca1, "c_rsa_ca1",       { "cn": "Client", "ip": "127.0.0.1", "dns": "client", "serial": 2 }),
    (ca1, "s_rsa_ca1_exp",   { "cn": "Server", "validity": -1 }), # Expired certificate
    (ca1, "c_rsa_ca1_exp",   { "cn": "Client", "validity": -1 }), # Expired certificate

    (ca1, "s_rsa_ca1_cn1",   { "cn": "Server", "dns": "localhost" }),       # DNS subjectAltName localhost
    (ca1, "s_rsa_ca1_cn2",   { "cn": "Server", "dns": "localhostXX" }),     # DNS subjectAltName localhostXX
    (ca1, "s_rsa_ca1_cn3",   { "cn": "localhost" }),                        # No subjectAltName, CN=localhost
    (ca1, "s_rsa_ca1_cn4",   { "cn": "localhostXX" }),                      # No subjectAltName, CN=localhostXX
    (ca1, "s_rsa_ca1_cn5",   { "cn": "localhost", "dns": "localhostXX" }),  # DNS subjectAltName localhostXX, CN=localhost
    (ca1, "s_rsa_ca1_cn6",   { "cn": "Server", "ip": "127.0.0.1" }),        # IP subjectAltName 127.0.0.1
    (ca1, "s_rsa_ca1_cn7",   { "cn": "Server", "ip": "127.0.0.2" }),        # IP subjectAltName 127.0.0.2
    (ca1, "s_rsa_ca1_cn8",   { "cn": "127.0.0.1" }),                        # No subjectAltName, CN=127.0.0.1

    (ca2, "s_rsa_ca2",       { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (ca2, "c_rsa_ca2",       { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }),
    (dsaca, "s_dsa_ca1",     { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }), # DSA
    (dsaca, "c_dsa_ca1",     { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }), # DSA
    (cai1, "s_rsa_cai1",     { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (cai2, "s_rsa_cai2",     { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (cai2, "c_rsa_cai2",     { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }),
]

#
# Create the certificates
#
for (ca, alias, args) in certs:
    if not ca.get(alias):
        ca.create(alias, **args)

savecerts = [
    (ca1, "s_rsa_ca1",     None,              {}),
    (ca1, "c_rsa_ca1",     None,              {}),
    (ca1, "s_rsa_ca1_exp", None,              {}),
    (ca1, "c_rsa_ca1_exp", None,              {}),
    (ca1, "s_rsa_ca1_cn1", None,              {}),
    (ca1, "s_rsa_ca1_cn2", None,              {}),
    (ca1, "s_rsa_ca1_cn3", None,              {}),
    (ca1, "s_rsa_ca1_cn4", None,              {}),
    (ca1, "s_rsa_ca1_cn5", None,              {}),
    (ca1, "s_rsa_ca1_cn6", None,              {}),
    (ca1, "s_rsa_ca1_cn7", None,              {}),
    (ca1, "s_rsa_ca1_cn8", None,              {}),
    (ca2, "s_rsa_ca2",     None,              {}),
    (ca2, "c_rsa_ca2",     None,              {}),
    (dsaca, "s_dsa_ca1",   None,              {}),
    (dsaca, "c_dsa_ca1",   None,              {}),
    (cai1, "s_rsa_cai1",   None,              {}),
    (cai2, "s_rsa_cai2",   None,              {}),
    (cai2, "c_rsa_cai2",   None,              {}),
    (ca1, "s_rsa_ca1",     "s_rsa_wroot_ca1", { "root": True }),
    (ca1, "s_rsa_ca1",     "s_rsa_pass_ca1",  { "password": "server" }),
    (ca1, "c_rsa_ca1",     "c_rsa_pass_ca1",  { "password": "client" }),
]

#
# Save the certificates in PEM and PKCS12 format.
#
for (ca, alias, path, args) in savecerts:
    if not path: path = alias
    password = args.get("password", None)
    cert = ca.get(alias)
    if force or not os.path.exists(path + "_pub.pem"):
        cert.save(path + "_pub.pem")
    if force or not os.path.exists(path + "_priv.pem"):
        cert.saveKey(path + "_priv.pem", password)
    if force or not os.path.exists(path + ".p12"):
        cert.save(path + ".p12", **args)

#
# Create DH parameters to use with macOS Secure Transport.
#
if clean or not os.path.exists("dh_params512.der"):
    ca1.run("openssl", "dhparam", 512, outform="DER", out="dh_params512.der")
if clean or not os.path.exists("dh_params1024.der"):
    ca1.run("openssl", "dhparam", 1024, outform="DER", out="dh_params1024.der")

#
# Create certificate with custom extensions
#
if not os.path.exists("cacert_custom.pem"):
    commands = ["openssl req -new -key cakey1.pem -out cacert_custom.csr -config cacert_custom.req",
                "openssl x509 -req -in cacert_custom.csr -signkey cakey1.pem -out cacert_custom.pem -extfile cacert_custom.ext"]
    for command in commands:
        if os.system(command) != 0:
            print "error running command `{0}'".format(command)
            sys.exit(1)

    if os.path.exists("cacert_custom.csr"):
        os.remove("cacert_custom.csr")
