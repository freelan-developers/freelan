# Certificate management

You may test for certificate revocation by revoking one certificate or the other issuing the following commands:

## To revoke the intermediate certificate

> openssl ca -revoke intermediate.crt
> openssl ca -gencrl -cert ca.crt -keyfile ca.key -out ca.crl -crldays 3650

## To revoke the final certificate

> openssl ca -revoke final.crt
> openssl ca -gencrl -cert intermediate.crt -keyfile intermediate.key -out intermediate.crl -crldays 3650
