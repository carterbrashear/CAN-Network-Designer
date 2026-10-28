# CAN Network Designer - Error Codes

## Severity Levels

The first digit of an error code determines its severity.

- **0:** No error
- **1-3:** Low severity
- **4-6:** Moderate severity
- **7-8:** High severity
- **9:** Critical severity

The ID is relative to where the error occured

- UI: user interface 
- DB: database
- UK: unkown
- PR: project 

## Example Errors

| ID | CODE | Description |
|---|---|---|
| UK | 501 | Unknown error |
| DB | 504 | Database connection failed |

## Usage

Use `GetError()` to retrieve the formatted error message.

For example:

`DB-504: Database connection failed`