# Generation of OpenAPI Models

The models present in [common_model](./common_model) and [nrf](./nrf) have been generated using OpenAPI Generator CLI. 

We use OpenAPI Generator CLI Version `v6.0.1`. The reason is that newer versions have issues with generating the `anyOf`
relations of the 3GPP models correctly.

Also in this version, we have the issue that `anyOf` inheritance does not work properly:
```
inheritance without use of 'discriminator.propertyName' has been deprecated in the 5.x release.
```

As the error message suggests, 3GPP does not add the `discriminator.propertyName` to handle inheritance accordingly.
Thus, many models are generated wrongly, especially the "nullable" ones (e.g. `AccessTypeRm`). In this case we have to manually fix these.

## Manual Steps After Generation

After generating the models (e.g., when supporting a new release), there are some mandatory manual steps. 

### Backport Changes
We follow the design decision that we should write necessary code directly inside the models, where they belong to 
increase encapsulation. Therefore, we have to manually backport these changes to the newly generated models. 
`git diff` should be enough to see what has been added (e.g., a `to_string` method).

Some of the changes are also fixes to wrongly-generated models, thus these have to be ported as well.

### Fix Namespace   
We use the namespace `oai::model::common` for the common models, but NF-specific namespaces for NF-specific messages, 
e.g., `oai::common::nrf`. Therefore, in the NF-specific models, we have to adapt the namespace accordingly for the common types.

## Model Generation

First, we create a `config.json` file to configure how the models should be generated:
```json
{
  "apiPackage" : "oai.api",
  "modelPackage": "oai.model.common",
  "helpersPackage": "oai.model.common.helpers"
}
```

Of course, when generating models for, e.g., NRF, the modelPackage should be `oai.model.nrf`. 

Then, we have to download the 3GPP OpenAPI specifications:
```
git clone https://forge.3gpp.org/rep/all/5G_APIs.git -b REL-16
```

Finally, we can generate the models, in this example we generate the common models (3GPP TS 29.571):
```
docker run --rm -v ${PWD}:/local openapitools/openapi-generator-cli:v6.0.1 generate -c /local/config.json -i /local/TS29571_CommonData.yaml -g cpp-pistache-server -o /local/out
```
