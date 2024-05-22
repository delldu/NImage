# openssl genrsa -out vision_private.key 2048
# openssl rsa -in vision_private.key -pubout -out vision_public.key

# -----BEGIN SIGNED MESSAGE-----
# CPU: AMD Ryzen 9 3900X 12-Core Processor, 24 processors
# Board: X570-A PRO (MS-7C37)
# GPU: NVIDIA GeForce RTX 2080 Ti, driver version = 11.070, memory size = 10985 M, 1 devices
# MAC: 00:d8:61:79:85:57
# -----BND SIGNED MESSAGE-----

# -----BEGIN PUBLIC KEY-----
# MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAv+6HyN8t22EIbsLbWL57
# +3dLDbU077ei0kC57iU6WHAePbavOqSCCYG6xxQXX1E6Ti0fJ1/Se3YC2QlL9Ksv
# P6sfVnoHkYUeXFye+trXrus/b40GcWbVtIK9/BuVZiCJUJzQNQ4eQVOTjs/tv0aZ
# /MtyhQ2jx/+UHCo49c3oDWCas1DaxkKoVf5P2saI7QYSk1gPuuBFd0lXXhcwsWMs
# Q66wLtZzUcmtL8gD/SDk+jJjl6bprAJYCILFqrVVsS6lxDjZleD9PRtJtjRRGFiG
# LMLj5vtUniF14O1EZbFv6ZnBXd3C0FhRHnzm5TLMGl4b38jiYb7uLH5qi+qKMJww
# fQIDAQAB
# -----END PUBLIC KEY-----


# -----BEGIN SIGNATURE-----
# iQEzBAEBCgAdFiEEq8S+Y45nuEV5gY9pySggClCDemwFAmZFKuIACgkQySggClCD
# emwChAgAl2vQMVJp0O0pDHQM1INA2Dim8sUdToxBJ4GtYt7og/nG/6tVoOKAT7Dn
# K4N/IRax51rCdIL75kcIN5weR6myV8ZnXxdABHcaYyZQjAL6Iz/gbJScxXrEJSlh
# NvcZmpM6e7wZI3Quhv5nY9BZ4PftPEH1raGsHPGSPhgdF5qKqLyj0D0Tb8/OQSNY
# c8zVryzFwT9/+HsKhazxdaMJ2hl3UV61HE/pcp7kwWsejF/aXpdkRYdoXA+FIWTD
# m942W83gJwTYEcpAWGvQlBYzyJZKuyGMjN97A5UgX7RTus07jm+5Iyts9JnxeXAN
# JZNUJMDhfzfjHybeCEGKpp3Sh7GYgg==
# =zaV7
# -----END SIGNATURE-----


sign()
{
	echo "-----BEGIN SIGNED MESSAGE-----"
	cat hw.txt
	echo "-----END SIGNED MESSAGE-----"
	echo
	cat vision_public.key
	echo
	echo "-----BEGIN SIGNATURE-----"
	openssl dgst -sha512 -sign vision_private.key hw.txt | base64
	echo "-----END SIGNATURE-----"
}

sign | base64 > hw.lic

# openssl dgst -sha512 -sign vision_private.key hw.txt > /tmp/sign.sha512
# openssl dgst -sha512 -verify vision_public.key -signature /tmp/sign.sha512 hw.txt
